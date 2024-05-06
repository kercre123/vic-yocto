#!/bin/sh

sysctl -w kernel.core_pattern=/var/tmp/core.%e.%p.%s.%t
