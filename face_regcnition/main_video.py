import cv2
import serial
import time
from simple_facerec import SimpleFacerec

# Encode faces from a folder
sfr = SimpleFacerec()
sfr.load_encoding_images("images/")

# Setup port série (modifiez le port selon votre configuration)
arduino = serial.Serial('COM3', 9600)  # Remplacez 'COM3' par le port série approprié
time.sleep(2)  # Attendez que la connexion série soit stable

# Nom de la personne autorisée pour ouvrir la porte
authorized_person = "mohamed"  # Remplacez par le nom enregistré pour l'accès

# Load Camera
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()  # Capture une image à partir de la caméra

    # Détecte les visages connus dans l'image
    face_locations, face_names = sfr.detect_known_faces(frame)

    for face_loc, name in zip(face_locations, face_names):
        y1, x2, y2, x1 = face_loc[0], face_loc[1], face_loc[2], face_loc[3]

        cv2.putText(frame, name, (x1, y1 - 10), cv2.FONT_HERSHEY_DUPLEX, 1, (0, 0, 200), 2)
        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 0, 200), 4)

        # Vérifie si la personne détectée est autorisée
        if name == authorized_person:
            print(f"Access granted for {name}")
            arduino.write(b'OPEN')  # Envoie la commande d'ouverture à l'Arduino
        else:
            print(f"Access denied for {name}")
            arduino.write(b'CLOSE')  # Envoie la commande de fermeture à l'Arduino

    cv2.imshow("Frame", frame)  # Affiche l'image avec les visages détectés

    key = cv2.waitKey(1)  # Attend une touche
    if key == 27:  # Si la touche échappement est pressée, quitte la boucle
        break

cap.release()  # Libère la caméra
cv2.destroyAllWindows()  # Ferme toutes les fenêtres d'affichage OpenCV

# Fermeture de la connexion série avec l'Arduino
arduino.close()
