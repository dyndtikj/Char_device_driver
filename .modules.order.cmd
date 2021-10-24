cmd_/home/maks/src/chardev_module/modules.order := {   echo /home/maks/src/chardev_module/mychrdev.ko; :; } | awk '!x[$$0]++' - > /home/maks/src/chardev_module/modules.order
