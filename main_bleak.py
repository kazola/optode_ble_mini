#!/usr/bin/env python3


import asyncio
from bleak import BleakClient


def notification_handler(_, data):
    print(ord(data.decode()))


async def main():
    mac = 'fe:7d:29:d8:7d:67'
    uuid_c = '00002324-0000-1000-8000-00805f9b34fb'
    uuid_w = '00002325-0000-1000-8000-00805f9b34fb'
    async with BleakClient(mac) as cli:
        print('connected')
        await cli.start_notify(uuid_c, notification_handler)
        await asyncio.sleep(2.0)
        await cli.stop_notify(uuid_c)
        print('disconnected')


async def show_chars():

    address = 'fe:7d:29:d8:7d:67'

    async with BleakClient(address) as client:
        print(f"Connected: {client.is_connected}")

        for service in client.services:
            print(f"[Service] {service}")
            for char in service.characteristics:
                if "read" in char.properties:
                    try:
                        value = bytes(await client.read_gatt_char(char.uuid))
                        print(
                            f"\t[Characteristic] {char} ({','.join(char.properties)}), Value: {value}"
                        )
                    except Exception as e:
                        print(
                            f"\t[Characteristic] {char} ({','.join(char.properties)}), Value: {e}"
                        )

                else:
                    value = None
                    print(
                        f"\t[Characteristic] {char} ({','.join(char.properties)}), Value: {value}"
                    )

                for descriptor in char.descriptors:
                    try:
                        value = bytes(
                            await client.read_gatt_descriptor(descriptor.handle)
                        )
                        print(f"\t\t[Descriptor] {descriptor}) | Value: {value}")
                    except Exception as e:
                        print(f"\t\t[Descriptor] {descriptor}) | Value: {e}")


if __name__ == "__main__":
    asyncio.run(main())
    # asyncio.run(show_chars())
