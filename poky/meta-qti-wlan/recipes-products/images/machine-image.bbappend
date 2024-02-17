# Additional non-open source packages to be put to the root filesystem.
# If product is specified try to include product inc otherwise include base inc.
def get_wlan_inc_file(d):
    product     = d.getVar('PRODUCT', True) or ""
    basemachine = d.getVar('BASEMACHINE', True)
    if product != 'base' or '':
        inc_file_name = basemachine + "-" + product + "-wlan-image.inc"
    else:
        inc_file_name = basemachine + "-wlan-image.inc"
    img_inc_file_path = os.path.join(d.getVar('THISDIR'), basemachine, inc_file_name)
    if os.path.exists(img_inc_file_path):
        img_inc_file = inc_file_name
    else:
        img_inc_file = basemachine + "-wlan-image.inc"
    return img_inc_file

include ${BASEMACHINE}/${@get_wlan_inc_file(d)}
