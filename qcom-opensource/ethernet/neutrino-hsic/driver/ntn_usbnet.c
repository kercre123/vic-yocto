/*
 * USB Network driver infrastructure
 * Copyright (C) 2000-2005 by David Brownell
 * Copyright (C) 2003-2005 David Hollis <dhollis@davehollis.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a generic "USB networking" framework that works with several
 * kinds of full and high speed networking devices:  host-to-host cables,
 * smart usb peripherals, and actual Ethernet adapters.
 *
 * These devices usually differ in terms of control protocols (if they
 * even have one!) and sometimes they define new framing to wrap or batch
 * Ethernet packets.  Otherwise, they talk to USB pretty much the same,
 * so interface (un)binding, endpoint I/O queues, fault handling, and other
 * issues can usefully be addressed by this framework.
 */

/* #define	DEBUG */		/* error path messages, extra info */
/* #define	VERBOSE */		/*  more; success messages */

#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ctype.h>
#include <linux/ethtool.h>
#include <linux/workqueue.h>
#include <linux/mii.h>
#include <linux/usb.h>
#include <linux/usb/usbnet.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/pm_runtime.h>
#define DRIVER_VERSION		"22-Aug-2005"


/*-------------------------------------------------------------------------*/

/*
 * Nineteen USB 1.1 max size bulk transactions per frame (ms), max.
 * Several dozen bytes of IPv4 data can fit in two such transactions.
 * One maximum size Ethernet packet takes twenty four of them.
 * For high speed, each frame comfortably fits almost 36 max size
 * Ethernet packets (so queues should be bigger).
 *
 * The goal is to let the USB host controller be busy for 5msec or
 * more before an irq is required, under load.  Jumbograms change
 * the equation.
 */
#define	MAX_QUEUE_MEMORY	(60 * 20480)
#define	RX_QLEN(dev)		((dev)->rx_qlen)
#define	TX_QLEN(dev)		((dev)->tx_qlen)

/* reawaken network queue this soon after stopping; else watchdog barks */
#define TX_TIMEOUT_JIFFIES	(5*HZ)

/* throttle rx/tx briefly after some faults, so hub_wq might disconnect()
 * us (it polls at HZ/4 usually) before we report too many false errors.
 */
#define THROTTLE_JIFFIES	(HZ/8)

/* between wakeups */
#define UNLINK_TIMEOUT_MS	3

/*-------------------------------------------------------------------------*/

/* randomly generated ethernet address */
static u8	node_id [ETH_ALEN];

static const char driver_name [] = "usbnet";

static struct workqueue_struct	*usbnet_wq;

/* use ethtool to change the level for any given device */
static int msg_level = -1;
/*-------------------------------------------------------------------------*/

static void intr_complete (struct urb *urb)
{
	struct usbnet	*dev = urb->context;
	int		status = urb->status;

	switch (status) {
	/* success */
	case 0:
		dev->driver_info->status(dev, urb);
		break;

	/* software-driven interface shutdown */
	case -ENOENT:		/* urb killed */
	case -ESHUTDOWN:	/* hardware gone */
		netif_dbg(dev, ifdown, dev->net,
			  "intr shutdown, code %d\n", status);
		return;

	/* NOTE:  not throttling like RX/TX, since this endpoint
	 * already polls infrequently
	 */
	default:
		printk(KERN_DEBUG "intr status %d\n", status);
		break;
	}

	status = usb_submit_urb (urb, GFP_ATOMIC);
	if (status != 0)
		netif_err(dev, timer, dev->net,
			  "intr resubmit --> %d\n", status);
}

static int init_status (struct usbnet *dev, struct usb_interface *intf)
{
	char		*buf = NULL;
	unsigned	pipe = 0;
	unsigned	maxp;
	unsigned	period;

	if (!dev->driver_info->status)
		return 0;

	pipe = usb_rcvintpipe (dev->udev,
			dev->status->desc.bEndpointAddress
				& USB_ENDPOINT_NUMBER_MASK);
	maxp = usb_maxpacket (dev->udev, pipe, 0);

	/* avoid 1 msec chatter:  min 8 msec poll rate */
	period = max ((int) dev->status->desc.bInterval,
		(dev->udev->speed == USB_SPEED_HIGH) ? 7 : 3);

	buf = kmalloc (maxp, GFP_KERNEL);
	if (buf) {
		dev->interrupt = usb_alloc_urb (0, GFP_KERNEL);
		if (!dev->interrupt) {
			kfree (buf);
			return -ENOMEM;
		} else {
			usb_fill_int_urb(dev->interrupt, dev->udev, pipe,
				buf, maxp, intr_complete, dev, period);
			dev->interrupt->transfer_flags |= URB_FREE_BUFFER;
			dev_dbg(&intf->dev,
				"status ep%din, %d bytes period %d\n",
				usb_pipeendpoint(pipe), maxp, period);
		}
	}
	return 0;
}

/* The caller must hold list->lock */
static void __usbnet_queue_skb(struct sk_buff_head *list,
			struct sk_buff *newsk, enum skb_state state)
{
	struct skb_data *entry = (struct skb_data *) newsk->cb;

	__skb_queue_tail(list, newsk);
	entry->state = state;
}

/*-------------------------------------------------------------------------*/

/* some LK 2.4 HCDs oopsed if we freed or resubmitted urbs from
 * completion callbacks.  2.5 should have fixed those bugs...
 */

static enum skb_state defer_bh(struct usbnet *dev, struct sk_buff *skb,
		struct sk_buff_head *list, enum skb_state state)
{
	unsigned long		flags;
	enum skb_state		old_state;
	struct skb_data *entry = (struct skb_data *) skb->cb;

	spin_lock_irqsave(&list->lock, flags);
	old_state = entry->state;
	entry->state = state;
	__skb_unlink(skb, list);
	spin_unlock(&list->lock);
	spin_lock(&dev->done.lock);
	__skb_queue_tail(&dev->done, skb);
	if (dev->done.qlen == 1)
		queue_work(usbnet_wq, &dev->bh_w);
	spin_unlock_irqrestore(&dev->done.lock, flags);
	return old_state;
}

/*-------------------------------------------------------------------------*/

static void ntn_rx_complete (struct urb *urb);

static int ntn_rx_submit (struct usbnet *dev, struct urb *urb, gfp_t flags,
		unsigned int pipe)
{
	struct sk_buff		*skb;
	struct skb_data		*entry;
	int			retval = 0;
	unsigned long		lockflags;
	size_t			size = dev->rx_urb_size;
	unsigned    index = 0;
	struct ntn_data *priv = NULL;

	/* printk(KERN_DEBUG "ntn_rx_submit pipe =%x\n", pipe); */
	priv = (struct ntn_data *)dev->data[0];

	if (priv == NULL) {
		return -ENOLINK;
	}

	if (pipe == priv->avb_in)
		index = AVB_PIPE_IDX;
	else if (pipe == priv->avb_control)
		index = AVBCTL_PIPE_IDX;
	else if (pipe == priv->gptp_in)
		index = GPTP_PIPE_IDX;
	else
		index = LEGACY_PIPE_IDX;

	/* prevent rx skb allocation when error ratio is high */
	if (test_bit(EVENT_RX_KILL, &dev->flags)) {
		usb_free_urb(urb);
		return -ENOLINK;
	}

	skb = __netdev_alloc_skb_ip_align(dev->net, size, flags);
	if (!skb) {
		netif_dbg(dev, rx_err, dev->net, "no rx skb\n");

		priv->kevent_pipe_flags |= (1 << index);
		usbnet_defer_kevent (dev, EVENT_RX_MEMORY);
		usb_free_urb (urb);
		return -ENOMEM;
	}

	entry = (struct skb_data *) skb->cb;
	entry->urb = urb;
	entry->dev = dev;
	entry->length = 0;

	usb_fill_bulk_urb (urb, dev->udev, pipe,
		skb->data, size, ntn_rx_complete, skb);

	spin_lock_irqsave (&dev->rxq.lock, lockflags);

	if (netif_running (dev->net) &&
	    netif_device_present (dev->net) &&
	    !test_bit (EVENT_RX_HALT, &dev->flags) &&
	    !test_bit (EVENT_DEV_ASLEEP, &dev->flags)) {
		switch (retval = usb_submit_urb (urb, GFP_ATOMIC)) {
		case -EPIPE:
			usbnet_defer_kevent (dev, EVENT_RX_HALT);
			break;
		case -ENOMEM:
			priv->kevent_pipe_flags |= (1 << index);
			usbnet_defer_kevent (dev, EVENT_RX_MEMORY);
			break;
		case -ENODEV:
			netif_dbg(dev, ifdown, dev->net, "device gone\n");
			netif_device_detach (dev->net);
			break;
		case -EHOSTUNREACH:
			retval = -ENOLINK;
			break;
		default:
			netif_dbg(dev, rx_err, dev->net,
				  "rx submit, %d\n", retval);
			queue_work(usbnet_wq, &dev->bh_w);
			break;
		case 0:
			__usbnet_queue_skb(&dev->rxq, skb, rx_start);
		}
	} else {
		netif_dbg(dev, ifdown, dev->net, "rx: stopped\n");
		retval = -ENOLINK;
	}
	spin_unlock_irqrestore (&dev->rxq.lock, lockflags);
	if (retval) {
		dev_kfree_skb_any (skb);
		usb_free_urb (urb);
	}
	return retval;
}


/*-------------------------------------------------------------------------*/

static inline void rx_process (struct usbnet *dev, struct sk_buff *skb)
{
	if (dev->driver_info->rx_fixup &&
	    !dev->driver_info->rx_fixup (dev, skb)) {
		/* With RX_ASSEMBLE, rx_fixup() must update counters */
		if (!(dev->driver_info->flags & FLAG_RX_ASSEMBLE))
			dev->net->stats.rx_errors++;
		goto done;
	}
	/* else network stack removes extra byte if we forced a short packet */

	/* all data was already cloned from skb inside the driver */
	if (dev->driver_info->flags & FLAG_MULTI_PACKET)
		goto done;

	if (skb->len < ETH_HLEN) {
		dev->net->stats.rx_errors++;
		dev->net->stats.rx_length_errors++;
		netif_dbg(dev, rx_err, dev->net, "rx length %d\n", skb->len);
	} else {
		usbnet_skb_return(dev, skb);
		return;
	}

done:
	skb_queue_tail(&dev->done, skb);
}

/*-------------------------------------------------------------------------*/

static void ntn_rx_complete (struct urb *urb)
{
	struct sk_buff		*skb = (struct sk_buff *) urb->context;
	struct skb_data		*entry = (struct skb_data *) skb->cb;
	struct usbnet		*dev = entry->dev;
	int			urb_status = urb->status;
	enum skb_state		state;

	skb_put (skb, urb->actual_length);
	state = rx_done;
	entry->urb = NULL;

	switch (urb_status) {
	/* success */
	case 0:
		break;

	/* stalls need manual reset. this is rare ... except that
	 * when going through USB 2.0 TTs, unplug appears this way.
	 * we avoid the highspeed version of the ETIMEDOUT/EILSEQ
	 * storm, recovering as needed.
	 */
	case -EPIPE:
		dev->net->stats.rx_errors++;
		usbnet_defer_kevent (dev, EVENT_RX_HALT);
		/* FALLTHROUGH */

	/* software-driven interface shutdown */
	case -ECONNRESET:		/* async unlink */
	case -ESHUTDOWN:		/* hardware gone */
		netif_dbg(dev, ifdown, dev->net,
			  "rx shutdown, code %d\n", urb_status);
		goto block;

	/* we get controller i/o faults during hub_wq disconnect() delays.
	 * throttle down resubmits, to avoid log floods; just temporarily,
	 * so we still recover when the fault isn't a hub_wq delay.
	 */
	case -EPROTO:
	case -ETIME:
	case -EILSEQ:
		dev->net->stats.rx_errors++;
		if (!timer_pending (&dev->delay)) {
			mod_timer (&dev->delay, jiffies + THROTTLE_JIFFIES);
			netif_dbg(dev, link, dev->net,
				  "rx throttle %d\n", urb_status);
		}
block:
		state = rx_cleanup;
		entry->urb = urb;
		urb = NULL;
		break;

	/* data overrun ... flush fifo? */
	case -EOVERFLOW:
		dev->net->stats.rx_over_errors++;
		/* FALLTHROUGH */

	default:
		state = rx_cleanup;
		dev->net->stats.rx_errors++;
		netif_dbg(dev, rx_err, dev->net, "rx status %d\n", urb_status);
		break;
	}

	/* stop rx if packet error rate is high */
	if (++dev->pkt_cnt > 30) {
		dev->pkt_cnt = 0;
		dev->pkt_err = 0;
	} else {
		if (state == rx_cleanup)
			dev->pkt_err++;
		if (dev->pkt_err > 20)
			set_bit(EVENT_RX_KILL, &dev->flags);
	}

	state = defer_bh(dev, skb, &dev->rxq, state);

	if (urb) {
		if (netif_running (dev->net) &&
		    !test_bit (EVENT_RX_HALT, &dev->flags) &&
		    state != unlink_start) {
			/* printk(KERN_DEBUG "ntn_rx_complete pipe =%x\n", urb->pipe); */
			ntn_rx_submit (dev, urb, GFP_ATOMIC, urb->pipe);
			usb_mark_last_busy(dev->udev);
			return;
		}
		usb_free_urb (urb);
	}
	netif_dbg(dev, rx_err, dev->net, "no read resubmitted\n");
}



static int unlink_urbs (struct usbnet *dev, struct sk_buff_head *q)
{
	unsigned long		flags;
	struct sk_buff		*skb;
	int			count = 0;

	spin_lock_irqsave (&q->lock, flags);
	while (!skb_queue_empty(q)) {
		struct skb_data		*entry;
		struct urb		*urb;
		int			retval;

		skb_queue_walk(q, skb) {
			entry = (struct skb_data *) skb->cb;
			if (entry->state != unlink_start)
				goto found;
		}
		break;
found:
		entry->state = unlink_start;
		urb = entry->urb;

		/*
		 * Get reference count of the URB to avoid it to be
		 * freed during usb_unlink_urb, which may trigger
		 * use-after-free problem inside usb_unlink_urb since
		 * usb_unlink_urb is always racing with .complete
		 * handler(include defer_bh).
		 */
		usb_get_urb(urb);
		spin_unlock_irqrestore(&q->lock, flags);
		/* during some PM-driven resume scenarios,
		 * these (async) unlinks complete immediately
		 */
		retval = usb_unlink_urb (urb);
		if (retval != -EINPROGRESS && retval != 0)
			printk(KERN_DEBUG "unlink urb err, %d\n", retval);
		else
			count++;
		usb_put_urb(urb);
		spin_lock_irqsave(&q->lock, flags);
	}
	spin_unlock_irqrestore (&q->lock, flags);
	return count;
}


/* drivers may override default ethtool_ops in their bind() routine */
static const struct ethtool_ops usbnet_ethtool_ops = {
	.get_settings		= usbnet_get_settings,
	.set_settings		= usbnet_set_settings,
	.get_link		= usbnet_get_link,
	.nway_reset		= usbnet_nway_reset,
	.get_drvinfo		= usbnet_get_drvinfo,
	.get_msglevel		= usbnet_get_msglevel,
	.set_msglevel		= usbnet_set_msglevel,
	.get_ts_info		= ethtool_op_get_ts_info,
};

/*-------------------------------------------------------------------------*/

static void __handle_link_change(struct usbnet *dev)
{
	if (!test_bit(EVENT_DEV_OPEN, &dev->flags))
		return;

	if (!netif_carrier_ok(dev->net)) {
		/* kill URBs for reading packets to save bus bandwidth */
		unlink_urbs(dev, &dev->rxq);

		/*
		 * tx_timeout will unlink URBs for sending packets and
		 * tx queue is stopped by netcore after link becomes off
		 */
	} else {
		/* submitting URBs for reading packets */
		queue_work(usbnet_wq, &dev->bh_w);
	}

	/* hard_mtu or rx_urb_size may change during link change */
	usbnet_update_max_qlen(dev);

	clear_bit(EVENT_LINK_CHANGE, &dev->flags);
}

static void usbnet_set_rx_mode(struct net_device *net)
{
	struct usbnet		*dev = netdev_priv(net);

	usbnet_defer_kevent(dev, EVENT_SET_RX_MODE);
}

static void __handle_set_rx_mode(struct usbnet *dev)
{
	if (dev->driver_info->set_rx_mode)
		(dev->driver_info->set_rx_mode)(dev);

	clear_bit(EVENT_SET_RX_MODE, &dev->flags);
}

/* work that cannot be done in interrupt context uses keventd.
 *
 * NOTE:  with 2.5 we could do more of this using completion callbacks,
 * especially now that control transfers can be queued.
 */
static void
kevent (struct work_struct *work)
{
	struct usbnet		*dev =
		container_of(work, struct usbnet, kevent);
	int			status;

	/* usb_clear_halt() needs a thread context */
	if (test_bit (EVENT_TX_HALT, &dev->flags)) {
		unlink_urbs (dev, &dev->txq);
		status = usb_autopm_get_interface(dev->intf);
		if (status < 0)
			goto fail_pipe;
		status = usb_clear_halt (dev->udev, dev->out);
		usb_autopm_put_interface(dev->intf);
		if (status < 0 &&
		    status != -EPIPE &&
		    status != -ESHUTDOWN) {
			if (netif_msg_tx_err (dev))
fail_pipe:
				printk(KERN_ERR "can't clear tx halt, status %d\n",
					   status);
		} else {
			clear_bit (EVENT_TX_HALT, &dev->flags);
			if (status != -ESHUTDOWN)
				netif_wake_queue (dev->net);
		}
	}
	if (test_bit (EVENT_RX_HALT, &dev->flags)) {
		unlink_urbs (dev, &dev->rxq);
		status = usb_autopm_get_interface(dev->intf);
		if (status < 0)
			goto fail_halt;
		status = usb_clear_halt (dev->udev, dev->in);
		usb_autopm_put_interface(dev->intf);
		if (status < 0 &&
		    status != -EPIPE &&
		    status != -ESHUTDOWN) {
			if (netif_msg_rx_err (dev))
fail_halt:
				printk(KERN_ERR "can't clear rx halt, status %d\n",
					   status);
		} else {
			clear_bit (EVENT_RX_HALT, &dev->flags);
			queue_work(usbnet_wq, &dev->bh_w);
		}
	}

	/* tasklet could resubmit itself forever if memory is tight */
	if (test_bit (EVENT_RX_MEMORY, &dev->flags)) {
		struct urb	*urb = NULL;
		int resched = 1;
		int j;
		unsigned pipe[4];
		struct ntn_data *priv = NULL;

		priv = (struct ntn_data *)dev->data[0];
		if (priv == NULL)
			return;

		pipe[LEGACY_PIPE_IDX] = priv->legacy_in;
		pipe[AVB_PIPE_IDX]    = priv->avb_in;
		pipe[AVBCTL_PIPE_IDX] = priv->avb_control;
		pipe[GPTP_PIPE_IDX]   = priv->gptp_in;

		for (j = 0; j < 4; j++) {
			if ((priv->kevent_pipe_flags & (1 << j)) == 0)
				continue;

			priv->kevent_pipe_flags &= ~(1 << j);
			if (netif_running (dev->net))
				urb = usb_alloc_urb (0, GFP_KERNEL);
			else
				clear_bit (EVENT_RX_MEMORY, &dev->flags);
			if (urb != NULL) {
				clear_bit (EVENT_RX_MEMORY, &dev->flags);
				status = usb_autopm_get_interface(dev->intf);
				if (status < 0) {
					usb_free_urb(urb);
					goto fail_lowmem;
				}

				if (ntn_rx_submit (dev, urb, GFP_KERNEL,
							pipe[j]) == -ENOLINK)
					resched = 0;
				usb_autopm_put_interface(dev->intf);
fail_lowmem:
				if (resched)
					queue_work(usbnet_wq, &dev->bh_w);
			}
		}
	}

	if (test_bit (EVENT_LINK_RESET, &dev->flags)) {
		struct driver_info	*info = dev->driver_info;
		int			retval = 0;

		clear_bit (EVENT_LINK_RESET, &dev->flags);
		status = usb_autopm_get_interface(dev->intf);
		if (status < 0)
			goto skip_reset;
		if(info->link_reset && (retval = info->link_reset(dev)) < 0) {
			usb_autopm_put_interface(dev->intf);
skip_reset:
			printk(KERN_INFO "link reset failed (%d) usbnet usb-%s-%s, %s\n",
				    retval,
				    dev->udev->bus->bus_name,
				    dev->udev->devpath,
				    info->description);
		} else {
			usb_autopm_put_interface(dev->intf);
		}

		/* handle link change from link resetting */
		__handle_link_change(dev);
	}

	if (test_bit (EVENT_LINK_CHANGE, &dev->flags))
		__handle_link_change(dev);

	if (test_bit (EVENT_SET_RX_MODE, &dev->flags))
		__handle_set_rx_mode(dev);


	if (dev->flags)
		printk(KERN_DEBUG "kevent done, flags = 0x%lx\n",
					dev->flags);
}


static int rx_alloc_submit(struct usbnet *dev, gfp_t flags)
{
	struct urb	*urb;
	int		i;
	int		ret = 0, error = 0;
	struct ntn_data *priv = NULL;

	priv = (struct ntn_data *)dev->data[0];
	if (priv == NULL) {
		ret = -ENOMEM;
		goto err;
	}

	/* don't refill the queue all at once */
	for (i = 0; i < 4*20 && dev->rxq.qlen < 4 * RX_QLEN(dev); i += 4) {
		/* legacy pipe */
		urb = usb_alloc_urb(0, flags);
		if (urb != NULL) {
			urb->pipe = priv->legacy_in;
			ret = ntn_rx_submit(dev, urb, flags, urb->pipe);
			if (ret) {
				error |= ret; /*goto err; */
			}
		} else {
			error = -ENOMEM;
			goto err;
		}
		/* gptp pipe */
		urb = usb_alloc_urb(0, flags);
		if (urb != NULL) {
			urb->pipe = priv->gptp_in;
			ret = ntn_rx_submit(dev, urb, flags, urb->pipe);
			if (ret)
				error |= ret; /* goto err; */
		} else {
			error = -ENOMEM;
			goto err;
		}

		/* avb pipe */
		urb = usb_alloc_urb(0, flags);
		if (urb != NULL) {
			urb->pipe = priv->avb_in;
			ret = ntn_rx_submit(dev, urb, flags, urb->pipe);
			if (ret)
				error |= ret; /* goto err; */
			if (dev->rxq.qlen == 0)
				goto err;
		} else {
			error = -ENOMEM;
			goto err;
		}

		/* avb_ctl pipe */
		urb = usb_alloc_urb(0, flags);
		if (urb != NULL) {
			urb->pipe = priv->avb_control;
			ret = ntn_rx_submit(dev, urb, flags, urb->pipe);
			if (ret)
				error |= ret; /* goto err; */
		} else {
			error = -ENOMEM;
			goto err;
		}
	}

err:
	return error;
}

/*-------------------------------------------------------------------------*/

/* tasklet (work deferred from completions, in_irq) or timer */

static void usbnet_bh (unsigned long param)
{
	struct usbnet		*dev = (struct usbnet *) param;
	struct sk_buff		*skb;
	struct skb_data		*entry;

	while ((skb = skb_dequeue (&dev->done))) {
		entry = (struct skb_data *) skb->cb;
		switch (entry->state) {
		case rx_done:
			entry->state = rx_cleanup;
			rx_process (dev, skb);
			continue;
		case tx_done:
			kfree(entry->urb->sg);
		case rx_cleanup:
			usb_free_urb (entry->urb);
			dev_kfree_skb (skb);
			continue;
		default:
			printk(KERN_DEBUG "bogus skb state %d\n", entry->state);
		}
	}

	/* restart RX again after disabling due to high error rate */
	clear_bit(EVENT_RX_KILL, &dev->flags);

	/* waiting for all pending urbs to complete?
	 * only then can we forgo submitting anew
	 */
	if (waitqueue_active(&dev->wait)) {
		if (dev->txq.qlen + dev->rxq.qlen + dev->done.qlen == 0)
			wake_up_all(&dev->wait);

	/* or are we maybe short a few urbs? */
	} else if (netif_running (dev->net) &&
		   netif_device_present (dev->net) &&
		   netif_carrier_ok(dev->net) &&
		   !timer_pending (&dev->delay) &&
		   !test_bit (EVENT_RX_HALT, &dev->flags)) {
		int	temp = dev->rxq.qlen;

		if (temp < RX_QLEN(dev)) {
			if (rx_alloc_submit(dev, GFP_KERNEL) == -ENOLINK)
				return;
			if (temp != dev->rxq.qlen)
				netif_dbg(dev, link, dev->net,
					  "rxqlen %d --> %d\n",
					  temp, dev->rxq.qlen);
			if (dev->rxq.qlen < RX_QLEN(dev))
				queue_work(usbnet_wq, &dev->bh_w);
		}
		if (dev->txq.qlen < TX_QLEN (dev))
			netif_wake_queue (dev->net);
	}
}

static void usbnet_bh_w(struct work_struct *work)
{
	struct usbnet		*dev =
		container_of(work, struct usbnet, bh_w);
	unsigned long param = (unsigned long)dev;

	usbnet_bh(param);
}

/*-------------------------------------------------------------------------
 *
 * USB Device Driver support
 *
 *-------------------------------------------------------------------------*/

static const struct net_device_ops usbnet_netdev_ops = {
	.ndo_open		= usbnet_open,
	.ndo_stop		= usbnet_stop,
	.ndo_start_xmit		= usbnet_start_xmit,
	.ndo_tx_timeout		= usbnet_tx_timeout,
	.ndo_set_rx_mode	= usbnet_set_rx_mode,
	.ndo_change_mtu		= usbnet_change_mtu,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

/*-------------------------------------------------------------------------*/

// precondition: never called in_interrupt

static struct device_type wlan_type = {
	.name	= "wlan",
};

static struct device_type wwan_type = {
	.name	= "wwan",
};
int
ntn_usbnet_probe (struct usb_interface *udev, const struct usb_device_id *prod)
{
	struct usbnet			*dev;
	struct net_device		*net;
	struct usb_host_interface	*interface;
	struct driver_info		*info;
	struct usb_device		*xdev;
	int				status;
	const char			*name;
	struct usb_driver	*driver = to_usb_driver(udev->dev.driver);
	/* usbnet already took usb runtime pm, so have to enable the feature
	 * for usb interface, otherwise usb_autopm_get_interface may return
	 * failure if RUNTIME_PM is enabled.
	 */
	if (!driver->supports_autosuspend) {
		driver->supports_autosuspend = 1;
		pm_runtime_enable(&udev->dev);
	}

	name = udev->dev.driver->name;
	info = (struct driver_info *) prod->driver_info;
	if (!info) {
		dev_dbg (&udev->dev, "blacklisted by %s\n", name);
		return -ENODEV;
	}
	xdev = interface_to_usbdev (udev);
	interface = udev->cur_altsetting;

	status = -ENOMEM;

	/* Need to destroy it in exit */
	usbnet_wq  = create_singlethread_workqueue("usbnet");
	if (!usbnet_wq) {
		pr_err("%s: Unable to create workqueue:usbnet\n", __func__);
		goto exit;
	}
	pr_err("%s: able to create workqueue:usbnet\n", __func__);

	/* set up our own records */
	net = alloc_etherdev(sizeof(*dev));
	if (!net)
		goto free_workqueue;

	/* netdev_printk() needs this so do it as early as possible */
	SET_NETDEV_DEV(net, &udev->dev);

	dev = netdev_priv(net);
	dev->udev = xdev;
	dev->intf = udev;
	dev->driver_info = info;
	dev->driver_name = name;
	dev->msg_enable = netif_msg_init (msg_level, NETIF_MSG_DRV
				| NETIF_MSG_PROBE | NETIF_MSG_LINK);
	init_waitqueue_head(&dev->wait);
	skb_queue_head_init (&dev->rxq);
	skb_queue_head_init (&dev->txq);
	skb_queue_head_init (&dev->done);
	skb_queue_head_init(&dev->rxq_pause);
	INIT_WORK(&dev->bh_w, usbnet_bh_w);
	INIT_WORK (&dev->kevent, kevent);
	init_usb_anchor(&dev->deferred);
	dev->delay.function = usbnet_bh;
	dev->delay.data = (unsigned long) dev;
	init_timer (&dev->delay);
	mutex_init (&dev->phy_mutex);
	mutex_init(&dev->interrupt_mutex);
	dev->interrupt_count = 0;
	dev->net = net;
	strlcpy (net->name, "usb%d", sizeof(net->name));
	memcpy (net->dev_addr, node_id, sizeof(node_id));

	/* rx and tx sides can use different message sizes;
	 * bind() should set rx_urb_size in that case.
	 */
	dev->hard_mtu = net->mtu + net->hard_header_len;
#if 0
	/* dma_supported() is deeply broken on almost all architectures
	 * possible with some EHCI controllers
	 */
	if (dma_supported (&udev->dev, DMA_BIT_MASK(64)))
		net->features |= NETIF_F_HIGHDMA;
#endif

	net->netdev_ops = &usbnet_netdev_ops;
	net->watchdog_timeo = TX_TIMEOUT_JIFFIES;
	net->ethtool_ops = &usbnet_ethtool_ops;

	/* allow device-specific bind/init procedures
	 * NOTE net->name still not usable ...
	 */
	if (info->bind) {
		status = info->bind (dev, udev);
		if (status < 0)
			goto free_netdevice;

		/* heuristic:  "usb%d" for links we know are two-host,
		 * else "eth%d" when there's reasonable doubt.  userspace
		 * can rename the link if it knows better.
		 */
		if ((dev->driver_info->flags & FLAG_ETHER) != 0 &&
		    ((dev->driver_info->flags & FLAG_POINTTOPOINT) == 0 ||
		     (net->dev_addr [0] & 0x02) == 0))
			strlcpy (net->name, "eth%d", sizeof(net->name));
		/* WLAN devices should always be named "wlan%d" */
		if ((dev->driver_info->flags & FLAG_WLAN) != 0)
			strlcpy(net->name, "wlan%d", sizeof(net->name));
		/* WWAN devices should always be named "wwan%d" */
		if ((dev->driver_info->flags & FLAG_WWAN) != 0)
			strlcpy(net->name, "wwan%d", sizeof(net->name));

		/* devices that cannot do ARP */
		if ((dev->driver_info->flags & FLAG_NOARP) != 0)
			net->flags |= IFF_NOARP;

		/* maybe the remote can't receive an Ethernet MTU */
		if (net->mtu > (dev->hard_mtu - net->hard_header_len))
			net->mtu = dev->hard_mtu - net->hard_header_len;
	} else if (!info->in || !info->out)
		status = usbnet_get_endpoints (dev, udev);
	else {
		dev->in = usb_rcvbulkpipe (xdev, info->in);
		dev->out = usb_sndbulkpipe (xdev, info->out);
		if (!(info->flags & FLAG_NO_SETINT))
			status = usb_set_interface (xdev,
				interface->desc.bInterfaceNumber,
				interface->desc.bAlternateSetting);
		else
			status = 0;

	}
	if (status >= 0 && dev->status)
		status = init_status (dev, udev);
	if (status < 0)
		goto unbind;

	if (!dev->rx_urb_size)
		dev->rx_urb_size = dev->hard_mtu;
	dev->maxpacket = usb_maxpacket (dev->udev, dev->out, 1);

	/* let userspace know we have a random address */
	if (ether_addr_equal(net->dev_addr, node_id))
		net->addr_assign_type = NET_ADDR_RANDOM;

	if ((dev->driver_info->flags & FLAG_WLAN) != 0)
		SET_NETDEV_DEVTYPE(net, &wlan_type);
	if ((dev->driver_info->flags & FLAG_WWAN) != 0)
		SET_NETDEV_DEVTYPE(net, &wwan_type);

	printk(KERN_DEBUG "urb size=%d\n", (int)dev->rx_urb_size);
	printk(KERN_DEBUG "net name size=%d\n", sizeof(net->name));
	/* initialize max rx_qlen and tx_qlen */
	usbnet_update_max_qlen(dev);
	/*  dev->rx_qlen = 10; */
	printk(KERN_DEBUG "len=%d, %d\n", dev->tx_qlen, dev->rx_qlen);

	if (dev->can_dma_sg && !(info->flags & FLAG_SEND_ZLP) &&
		!(info->flags & FLAG_MULTI_PACKET)) {
		dev->padding_pkt = kzalloc(1, GFP_KERNEL);
		if (!dev->padding_pkt) {
			status = -ENOMEM;
			goto free_urb;
		}
	}

	status = register_netdev (net);
	if (status)
		goto free_padding_pkt;
	netif_info(dev, probe, dev->net,
		   "register '%s' at usb-%s-%s, %s, %pKM\n",
		   udev->dev.driver->name,
		   xdev->bus->bus_name, xdev->devpath,
		   dev->driver_info->description,
		   net->dev_addr);

	/* ok, it's ready to go. */
	usb_set_intfdata (udev, dev);

	netif_device_attach (net);

	if (dev->driver_info->flags & FLAG_LINK_INTR)
		usbnet_link_change(dev, 0, 0);

	return 0;
free_padding_pkt:
	kfree(dev->padding_pkt);
free_urb:
	usb_free_urb(dev->interrupt);
unbind:
	if (info->unbind)
		info->unbind (dev, udev);
free_netdevice:
	free_netdev(net);
free_workqueue:
	destroy_workqueue(usbnet_wq);
exit:
	return status;
}


/* precondition: never called in_interrupt */

void
ntn_usbnet_disconnect (struct usb_interface *intf)
{
	struct usbnet		*dev;
	struct usb_device	*xdev;
	struct net_device	*net;
	int retval = 0;

	dev = usb_get_intfdata(intf);
	usb_set_intfdata(intf, NULL);
	if (!dev)
		return;

	xdev = interface_to_usbdev (intf);

	netif_info(dev, probe, dev->net, "unregister '%s' usb-%s-%s, %s\n",
		   intf->dev.driver->name,
		   xdev->bus->bus_name, xdev->devpath,
		   dev->driver_info->description);

	net = dev->net;
	unregister_netdev (net);

	cancel_work_sync(&dev->kevent);
	usb_scuttle_anchored_urbs(&dev->deferred);

	if (dev->driver_info->unbind)
		dev->driver_info->unbind (dev, intf);

	usb_kill_urb(dev->interrupt);
	usb_free_urb(dev->interrupt);

	kfree(dev->padding_pkt);

	free_netdev(net);
	destroy_workqueue(usbnet_wq);
}
