import serial
import serial.tools.list_ports
import time
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import cv2

def main():
    imshape = (24, 32, 3)
    imdata = np.empty(imshape)
    plt.ion()
    fig, ax = plt.subplots()
    im = ax.imshow(imdata,cmap='plasma',origin ='lower')
    #plt.colorbar(im)
    while True:
        count = ser.inWaiting()  # 获取串口缓冲区数据
        if count != 0:
            recv = ser.readline().decode()[:-4]
            data = np.array(recv.split(',')).astype(np.float32)
            if len(data) == 768:
                heat = data.reshape((24, 32))
                print(heat)

                max = -40
                min = 500
                max_i = 0
                max_j = 0
                for i in range(24):
                    for j in range(32):
                        if heat[i, j] < min:
                            min = heat[i, j]
                        if heat[i, j] > max:
                            max = heat[i, j]
                            max_i = i
                            max_j = j
                heat[max_i, max_j] = 300
                if (max-min) > 10:
                    imdata = (heat - min) / (max - min)
                else:
                    imdata = (heat - min) / 10
                im.set_data(imdata)
                fig.canvas.draw()
                print(fig.canvas)
                fig.canvas.flush_events()
                ax.set_title("Max Temp:" + "%.2f"%max)

if __name__ == '__main__':
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        print(p)
        if "STMicroelectronics Virtual COM Port" in str(p):
            p = str(p)[-6:-1]
            print(p)
    ser = serial.Serial(p, 115200, timeout=5)  # 开启com3口，波特率115200，超时5
    ser.rts = False
    ser.flushInput()  # 清空缓冲区
    main()