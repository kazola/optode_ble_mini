from bluepy.btle import Scanner, DefaultDelegate, Peripheral


class MyDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleNotification(self, _, b):
        b = ord(b.decode())
        print(b)


def enable_notify(per,  chara_uuid):
    setup_data = b'\x01\x00'
    notify = per.getCharacteristics(uuid=chara_uuid)[0]
    notify_handle = notify.getHandle() + 1
    per.writeCharacteristic(notify_handle, setup_data, withResponse=True)


def scan():
    scanner = Scanner()
    devices = scanner.scan(5.0)
    for dev in devices:
        s = 'mac {} rssi {} -> {}'
        print(s.format(dev.addr, dev.rssi, dev.getScanData()))


if __name__ == '__main__':
    # while 1:
    #     scan()
    #     time.sleep(10)
    #     print('\n\n\n\n\n\n')

    p = Peripheral('72:51:d2:5f:7f:b3')
    p.setDelegate(MyDelegate())
    svc = p.getServiceByUUID('180F')
    enable_notify(p, '2A19')
    while 1:
        p.waitForNotifications(1.0)


