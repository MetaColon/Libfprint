package de.teemze;

import org.apache.commons.lang3.ArrayUtils;
import org.usb4java.*;

import javax.xml.bind.DatatypeConverter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
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
    private static final String RESULT_FILE_PATH = "image";
    private static final String TYPE = "pgm";
    private static final byte PADDING = 2; //TODO Evaluate good padding

    private static byte frameWidth;
    private static byte rawFrameWidth;
    private static byte frameHeight;

    private static int currentImageIndex = 0;
    private static Context context;
    private static Device device;
    private static DeviceHandle handle;

    public static void main(String[] args)
    {
        //saveImage(createTestImage());
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
                ByteBuffer dims = executeCommand("Activate GetSensorDim", new byte[]{0x00, 0x0c}, 4, false);
                processDimensions(dims);
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
                ByteBuffer captureStartResponse = executeCommand("Capture Start", new byte[]{0x40, 0x31}, 0, false);
                char cont;
                do
                {
                    ByteBuffer captureWaitResponse;
                    do
                    {
                        captureWaitResponse = executeCommand("Capture WaitFinger", new byte[]{0x40, 0x3f}, 1, false); //TODO
                    } while (captureWaitResponse.get(0) != 0x55);
                    ByteBuffer captureImage = executeCommand("Capture Read", new byte[]{0x00, 0x09}, -1, true);
                    saveImage(captureImage);

                    System.out.println("Continue? (y/n)");
                    cont = (char) System.in.read();
                    System.in.read();
                    currentImageIndex++;
                }while (Character.toLowerCase(cont) == 'y');

                // Deactivate
                ByteBuffer deactivateResponse = executeCommand("Deactivate", new byte[]{0x00, 0x0b}, 0, false);
            } catch (IOException e)
            {
                e.printStackTrace();
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

    private static void processDimensions (ByteBuffer rawData)
    {
        // See elan.c line 540ff
        frameWidth = rawData.get(2);
        rawFrameWidth = rawData.get(0);
        frameHeight = (byte)(rawFrameWidth - 2 * PADDING);
        System.out.println(String.format("frameWidth: %d; rawFrameWidth: %d; frameHeight: %d", frameWidth, rawFrameWidth, frameHeight));
    }

    /*private static ByteBuffer createTestImage()
    {
        ByteBuffer buffer = BufferUtils.allocateByteBuffer(WIDTH * HEIGHT);
        Random random = new Random();
        byte[] randomArray = new byte[WIDTH * HEIGHT];
        random.nextBytes(randomArray);
        for (int i = 0; i < WIDTH * HEIGHT; i++)
            buffer.put(i, randomArray[i]);
        return buffer;
    }*/

    private static boolean saveImage(ByteBuffer rawImage)
    {
        // See elan.c line 90ff
        /*byte rawHeight = frameWidth;
        byte rawWidth = rawFrameWidth;*/
        byte[] frame = new byte[64 * 144 / 2];

        /*for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 144; x++)
            {
                int frameIndex = y + x * 64;
                int rawIndex = x + y * 144;
                int index = y * 144 + x;
                frame[index] = rawImage.get(index);
                System.out.print(rawImage.get(index) > 0 ? "#" : " ");
            }
            System.out.println();
        }*/

        for (int i = 0; i < 144 * 64 / 2; i++) {
            frame[i] = rawImage.get(i * 2);
            System.out.print(frame[i] > 0 ? "#" : " ");
            if (i % 64 == 0)
                System.out.println();
        }

        File file = new File(String.format("%s_%d.%s", RESULT_FILE_PATH, currentImageIndex, TYPE));
        //file.deleteOnExit();
        try
        {
            // See img.c line 139ff
            FileOutputStream fileOutputStream = new FileOutputStream(file);
            char[] charArray = String.format("P5 %d %d 255\n", 144 / 2, 64).toCharArray();
            byte[] byteArray = new byte[charArray.length];
            for (int i = 0; i < charArray.length; i++)
                byteArray[i] = (byte) charArray[i];
            fileOutputStream.write(byteArray);
            fileOutputStream.write(frame);
            fileOutputStream.close();
            return true;
        } catch (IOException e)
        {
            e.printStackTrace();
            return false;
        }
    }

    private static ByteBuffer executeCommand(String name, byte[] data, int resultLength, boolean image)
    {
        System.out.println(name);
        write(data);
        System.out.print("    ");
        for (byte aData : data) {
            System.out.print(DatatypeConverter.printHexBinary(new byte[]{aData}) + ";");
        }
        System.out.println();
        //if (resultLength <= 0 && !image)
        //    return null;
        ByteBuffer result = image ? readImage() : read(resultLength);
        System.out.print("    ");
        for (int i = 0; i < resultLength; i++) {
            System.out.print(DatatypeConverter.printHexBinary(new byte[]{ result.get(i) } )+ ";");
        }
        System.out.println();
        return result;
    }

    private static boolean write(byte[] data)
    {
        ByteBuffer buffer = BufferUtils.allocateByteBuffer(data.length);
        buffer.put(data);
        IntBuffer transferred = BufferUtils.allocateIntBuffer();
        int result = LibUsb.bulkTransfer(handle, OUT_ENDPOINT, buffer, transferred, TIMEOUT);
        System.out.println("Sent " + transferred.get() + " bytes; Result: " + result);
        return result == LibUsb.SUCCESS;
    }

    private static ByteBuffer read(int size)
    {
        ByteBuffer buffer = BufferUtils.allocateByteBuffer(size).order(ByteOrder.LITTLE_ENDIAN);
        IntBuffer transferred = BufferUtils.allocateIntBuffer();
        int result = LibUsb.bulkTransfer(handle, IN_ENDPOINT, buffer, transferred, TIMEOUT);
        System.out.println("Read " + transferred.get() + " bytes; Result: " + result);
        if (result != LibUsb.SUCCESS)
            return null;
        return buffer;
    }

    private static ByteBuffer readImage()
    {
        ByteBuffer buffer = BufferUtils.allocateByteBuffer(/*rawFrameWidth * frameWidth & 0xFF * 2*/144 * 64).order(ByteOrder.LITTLE_ENDIAN);
        IntBuffer transferred = BufferUtils.allocateIntBuffer();
        int result = LibUsb.bulkTransfer(handle, IMG_ENDPOINT, buffer, transferred, TIMEOUT);
        System.out.println("Read " + transferred.get() + " bytes; Result: " + result);
        if (result != LibUsb.SUCCESS)
            return null;
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
