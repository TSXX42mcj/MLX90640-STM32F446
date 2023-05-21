import serial
import serial.tools.list_ports
import time
import numpy as np
import cv2
from matplotlib import cm
from scipy.ndimage.interpolation import zoom

def pseudocolor(val, minval, maxval):
    val = (val - minval) / (maxval - minval)
    return cm.jet(val)# Return RGBA tuple from jet colormap

def draw_cross(img, col, rol):
    cv2.line(img, (rol-4, col), (rol+4, col), (255, 255, 255), 1)
    cv2.line(img, (rol, col-4), (rol, col+4), (255, 255, 255), 1)

def main():
    while True:
        count = ser.inWaiting()  # 获取串口缓冲区数据
        if count != 0:
            recv = ser.readline().decode()[:-4]
            data = np.array(recv.split(',')).astype(np.float32)
            if len(data) == 773:
                max_range = data[768]
                min_range = data[769]
                max_temp = data[770]
                col = int(data[771]*9)
                rol = int(data[772]*9)
                heat = data[0:768].reshape((24, 32))

                cv2.namedWindow('Img', 0)
                cv2.resizeWindow('Img', 640, 480)
                heat = zoom(heat, 3)
                heat = zoom(heat, 3)
                heat = pseudocolor(heat, max_range, min_range)
                draw_cross(heat, col, rol)
                cv2.putText(heat, str(max_temp), (rol+4, col-4), cv2.FONT_HERSHEY_PLAIN, 1, (255, 255, 255), 1)
                # cv2.imwrite('temp.jpg', heat*255)
                cv2.imshow('Img', heat)
                cv2.waitKey(1)

if __name__ == '__main__':
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        print(p)
        if "STMicroelectronics Virtual COM Port" in str(p):
            ser = serial.Serial(str(p)[-6:-1], 921600, timeout=5)  # 波特率921600，超时5
            ser.rts = False
            ser.flushInput()  # 清空缓冲区
        main()