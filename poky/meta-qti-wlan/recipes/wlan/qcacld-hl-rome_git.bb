include qcacld-hl_git.bb

# Targets - mdm9650 single image, add new driver image: module name - wlan_sdio_rome.ko, chip name - qca6574
# for other target, need to override them to other values
python __anonymous () {
     if d.getVar('BASEMACHINE', True) == 'mdm9650':
         d.setVar('WLAN_MODULE_NAME', 'wlan_sdio_rome')
         d.setVar('CHIP_NAME', 'qca6574')

     if d.getVar('BASEMACHINE', True) == 'apq8009':
         d.setVar('WLAN_MODULE_NAME', 'wlan_rome')
         d.setVar('CHIP_NAME', 'qca6174')
}
