package de.teemze;

import org.usb4java.*;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;

public class Main
{
    private static final short VENDOR_ID = 0x04f3;
    private static final short PRODUCT_ID = 0x0c1a;
    private static final int TIMEOUT = 5000;
    private static final byte OUT_ENDPOINT = 0x01;
    private static final byte IN_ENDPOINT = (byte) 0x83;
    private static final byte IMG_ENDPOINT = (byte) 0x82;
    private static final int WIDTH = 144;
    private static final int HEIGHT = 64;

    private static Context context;
    private static Device device;
    private static DeviceHandle handle;

    public static void main(String[] args)
    {
        if (!initialize() || !findDevice() || !getHandle())
        {
            System.out.println("Something went wrong.");
            return;
        }
        try
        {
            if (LibUsb.claimInterface(handle, 0) != LibUsb.SUCCESS)
            {
                System.out.println("Couldn't claim interface.");
                return;
            }
            try
            {
                // Activate
                ByteBuffer dims = executeCommand("Activate GetSensorDim", new byte[]{0x00, 0x0c}, 4, false); //TODO use this dimensions
                ByteBuffer startInitResponse0 = executeCommand("Activate InitStart0", new byte[]{0x40, 0x19}, 2, false);
                ByteBuffer startInitResponse1 = executeCommand("Activate InitStart1", new byte[]{0x40, 0x2a}, 2, false); //TODO use the results
                ByteBuffer activateImage = executeCommand("Activate Read", new byte[]{0x00, 0x09}, -1, true);
                ByteBuffer endInitResponse = executeCommand("Activate InitEnd", new byte[]{0x40, 0x24}, 2, false);

                // Calibrate
                ByteBuffer calibrateStartResponse0 = executeCommand("Calibrate Start0", new byte[]{0x40, 0x23}, 1, false);
                ByteBuffer calibrateStartResponse1 = executeCommand("Calibrate Start1", new byte[]{0x40, 0x23}, 1, false);
                ByteBuffer calibrateImage = executeCommand("Calibrate Read", new byte[]{0x00, 0x09}, -1, true);
                ByteBuffer calibrateEndResponse = executeCommand("Calibrate End", new byte[]{0x40, 0x24}, 2, false);

                // Capture
                ByteBuffer captureStartResponse = executeCommand("Capture Start", new byte[]{0x40, 0x31}, 0, false); //TODO return value irrelevant
                ByteBuffer captureWaitResponse;
                do
                {
                    captureWaitResponse = executeCommand("Capture WaitFinger", new byte[]{0x40, 0x3f}, 1, false);
                } while (captureWaitResponse.get(0) != 0x55);
                ByteBuffer captureImage = executeCommand("Capture Read", new byte[]{0x00, 0x09}, -1, true); //TODO save image, get multiple images

                // Deactivate
                ByteBuffer deactivateResponse = executeCommand("Deactivate", new byte[]{0x00, 0x0b}, 0, false);
            } finally
            {
                if (LibUsb.releaseInterface(handle, 0) != LibUsb.SUCCESS)
                    System.out.println("Coulnd't release interface.");
            }
        } finally
        {
            LibUsb.close(handle);
        }
    }

    private static ByteBuffer executeCommand (String name, byte[] data, int resultLength, boolean image)
    {
        System.out.println(name);
        write(data);
        if (resultLength <= 0 && !image)
            return null;
        ByteBuffer result = image ? readImage() : read(resultLength);
        System.out.print("    ");
        System.out.println(result);
        return result;
    }

    private static boolean write(byte[] data)
    {
        ByteBuffer buffer = BufferUtils.allocateByteBuffer(data.length);
        buffer.put(data);
        IntBuffer transferred = BufferUtils.allocateIntBuffer();
        int result = LibUsb.bulkTransfer(handle, OUT_ENDPOINT, buffer, transferred, TIMEOUT);
        System.out.println("Sent " + transferred.get() + " bytes");
        return result == LibUsb.SUCCESS;
    }

    private static ByteBuffer read(int size)
    {
        ByteBuffer buffer = BufferUtils.allocateByteBuffer(size).order(ByteOrder.LITTLE_ENDIAN);
        IntBuffer transferred = BufferUtils.allocateIntBuffer();
        if (LibUsb.bulkTransfer(handle, IN_ENDPOINT, buffer, transferred, TIMEOUT) != LibUsb.SUCCESS)
            return null;
        System.out.println("Read " + transferred.get() + " bytes");
        return buffer;
    }

    private static ByteBuffer readImage ()
    {
        ByteBuffer buffer = BufferUtils.allocateByteBuffer(HEIGHT * WIDTH).order(ByteOrder.LITTLE_ENDIAN);
        IntBuffer transferred = BufferUtils.allocateIntBuffer();
        if (LibUsb.bulkTransfer(handle, IMG_ENDPOINT, buffer, transferred, TIMEOUT) != LibUsb.SUCCESS)
            return null;
        System.out.println("Read " + transferred.get() + " bytes");
        return buffer;
    }

    private static boolean initialize()
    {
        context = new Context();
        int initResult = LibUsb.init(context);
        return initResult == LibUsb.SUCCESS;
    }

    private static boolean findDevice()
    {
        // Read the USB device list
        DeviceList list = new DeviceList();
        int result = LibUsb.getDeviceList(null, list);
        if (result < 0) throw new LibUsbException("Unable to get device list", result);

        try
        {
            // Iterate over all devices and scan for the right one
            for (Device device : list)
            {
                DeviceDescriptor descriptor = new DeviceDescriptor();
                result = LibUsb.getDeviceDescriptor(device, descriptor);
                if (result != LibUsb.SUCCESS)
                    return false;
                if (descriptor.idVendor() == VENDOR_ID && descriptor.idProduct() == PRODUCT_ID)
                {
                    Main.device = device;
                    return true;
                }
            }
        } finally
        {
            // Ensure the allocated device list is freed
            LibUsb.freeDeviceList(list, true);
        }

        // Device not found
        return false;
    }

    private static boolean getHandle()
    {
        handle = new DeviceHandle();
        return LibUsb.open(device, handle) == LibUsb.SUCCESS;
    }
}
