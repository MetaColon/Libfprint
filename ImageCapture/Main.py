import usb1

VENDOR_ID = 0x04f3
PRODUCT_ID = 0x0c1a
INTERFACE = 0
WIDTH = 144
HEIGHT = 64 #validate
USB_OUT = 0x0
USB_IN = 0x80
CMD_OUT = 0x1 | USB_OUT
CMD_IN = 0x3 | USB_IN
IMG_IN = 0x2 | USB_IN
PRE_INIT = 0x4019
INIT = 0x4031
GET_IMAGE = 0x0009
GET_PRE_SCAN = 0x403f
STOP = 0x000b


def init(h):
    h.bulkWrite(CMD_OUT, PRE_INIT)
    h.bulkWrite(CMD_OUT, INIT)
    return


def getPreScan(h):
    h.bulkWrite(CMD_OUT, )
    return


def readPreScan(h):
    res = h.bulkRead(CMD_IN, 1)
    return res[0]#TODO Array? -> How to receive data?


def getImage(h):
    h.bulkWrite(CMD_OUT, GET_IMAGE)
    return


def readImage(h):
    res = h.bulkRead(IMG_IN, WIDTH * HEIGHT)
    return res#TODO What is res?


def stop(h):
    h.bulkWrite(CMD_OUT, STOP)
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
        index = 0
        while True:
            init(handle)
            getPreScan(handle)
            if readPreScan(handle) != 0x55:
                continue
            getImage(handle)
            img = readImage(handle)
            with open('image' + str(index) + '.pgm', 'wb') as f:#todo file ending
                f.write(img)#validate

            if input("Continue? (y/n):") != "y":
                break
        stop(handle)
        exit(0)
