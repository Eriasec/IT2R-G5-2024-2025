#========================== Programme Base IA ==== HD 2024 ======================================

import sensor, image, lcd
import KPU as kpu



#---------- Initialisation LCD et Caméra -----------
lcd.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.run(1)



#---------- Initialisation KPU avec le modèle IA entrainé au préalable -------------------------

modele = kpu.load(0x300000)   # Chargement des coefficients du modèle depuis l'adresse 0x300000

anchors = (1.889, 2.5245, 2.9465, 3.94056, 3.99987, 5.3658, 5.155437, 6.92275, 6.718375, 9.01025)

kpu.init_yolo2(modele, 0.5, 0.3, 5, anchors) # Initialisation du calculateur IA



#-------- Boucle principale : Capture image -> Calcul IA -> Affichage résultats ---------------

while True:
        img = sensor.snapshot()  # Capture d'une image
        Resultats = kpu.run_yolo2(modele, img)  # Récupération du résultat du calculateur IA
        if Resultats :
            for i in Resultats :  # il peut y avoir plusieurs identifications
                img.draw_rectangle(i.rect(),color=(0,255,0)) # Tracé de la boite englobante
                print(i)    # Sur terminal série, affichage du contenu de chaque résultat
        lcd.display(img)

