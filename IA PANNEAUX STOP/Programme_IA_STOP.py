import sensor, image, lcd
import KPU as kpu
from fpioa_manager import fm    # fpioa_manager: abbreviated as fm
from Maix import GPIO
import time

fm.register(13, fm.fpioa.GPIO0) # Attribue la pin IO14 à la fonction logicielle GPIO0

gpio = GPIO(GPIO.GPIO0, GPIO.OUT) # Création de l'objet "led_rouge" de classe GPIO

lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.run(1)
sensor.set_windowing((224,224))  # Ajout HD pour redimensionner les images du capteur

modele = kpu.load(0x300000)
anchors = [3.0, 3.0, 5.0, 6.0, 3.0, 3.0, 4.0, 4.0, 5.0, 3.0]
kpu.init_yolo2(modele, 0.5, 0.3, 5, anchors)

while True:
        img = sensor.snapshot()
        Resultats = kpu.run_yolo2(modele, img)
        if Resultats :
            for i in Resultats :
                print(i)
                img.draw_rectangle(i.rect(),color=(0,255,0))
                img.draw_string(i.x(), i.y(), str(int((i.value())*100)) + " % Panneau STOP détecté!!", lcd.GREEN, scale=2)
                gpio.value(1)
                time.sleep(0.2)
        else:
            gpio.value(0)
        lcd.display(img)




