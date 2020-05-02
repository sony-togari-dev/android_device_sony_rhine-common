#!/sbin/sh

strings /dev/block/bootdevice/by-name/LTALabel | grep -q c6[89]02
if [ $? -eq 0 ]; then
    for i in $(ls /mnt/system/system/etc/firmware/c6x02/); do
        mv /mnt/system/system/etc/firmware/c6x02/$i /mnt/system/system/etc/firmware/
    done
fi
rm -rf /mnt/system/system/etc/firmware/c6x02/
