import usb1

VENDOR_ID = 0x04f3
PRODUCT_ID = 0x0c1a
INTERFACE = 0
WIDTH = 144
HEIGHT = 64  # validate
USB_OUT = 0x0
USB_IN = 0x80
CMD_OUT = 0x1 | USB_OUT
CMD_IN = 0x3 | USB_IN
IMG_IN = 0x2 | USB_IN
PRE_INIT = bytes([0x40, 0x19])
INIT = bytes([0x40, 0x31])
GET_IMAGE = bytes([0x00, 0x09])
GET_PRE_SCAN = bytes([0x40, 0x3f])
STOP = bytes([0x00, 0x0b])


def init(h):
    h.bulkWrite(CMD_OUT, PRE_INIT)
    print("Wrote PreInit")
    h.bulkWrite(CMD_OUT, INIT)
    print("Wrote Init")
    return


def getPreScan(h):
    h.bulkWrite(CMD_OUT, GET_PRE_SCAN)
    print("Wrote GetPreScan")
    return


def readPreScan(h):
    print("Began to read prescan")
    res = h.bulkRead(CMD_IN, 1)
    print("Read PreScan")
    return res[0]  # TODO Array? -> How to receive data?


def getImage(h):
    h.bulkWrite(CMD_OUT, GET_IMAGE)
    print("Wrote GetImage")
    return


def readImage(h):
    print("Image size: " + str(WIDTH * HEIGHT))
    res = h.bulkRead(IMG_IN, WIDTH * HEIGHT)
    print("Read image: " + str(res))
    return res  # TODO What is res?


def stop(h):
    h.bulkWrite(CMD_OUT, STOP)
    print("Wrote stop")
    return


with usb1.USBContext() as context:
    handle = context.openByVendorIDAndProductID(
        VENDOR_ID,
        PRODUCT_ID,
        skip_on_error=True,
    )
    if handle is None:
        print("Couldn't connect to device")
        exit(1)
    with handle.claimInterface(INTERFACE):
        print("Got handle")
        index = 0
        while True:
            init(handle)
            getPreScan(handle)
            #if readPreScan(handle) != 0x55:
            #    continue
            getImage(handle)
            img = readImage(handle)
            with open('image' + str(index) + '.pgm', 'wb') as f:  # todo file ending
                print("Wrote image to " + str(f))
                f.write(img)  # validate

            if input("Continue? (y/n):") != "y":
                break
            index = index + 1
        stop(handle)
        exit(0)
