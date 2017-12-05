# Tetris Game

Scopul jocului este de a aduna cat mai multe puncte prin eliminarea liniilor orizontale. 
Liniile orizontale sunt eliminate atunci cand sunt umplute cu puncte. Utilizatorul are la dispozitie doar obiectele predefinite 
care cad cu o anumita viteza. 

## Reguli

Jocul incepe cu inputul userului. Acesta cu ajutorul joystick-ului poate muta obiectele ce cad, in stanga sau dreapta pentru a 
umple liniile orizontale ale matricei. In cazul in care o linie este umpluta, ea va disparea.
Userul are 4 vieti; userul pierde o viata in momentul in care se formeaza o coloana completa de puncte si 
se elimina ultimele 2 randuri ale matricei pentru a se putea continua jocul.
Jocul e construit pe nivele de dificultate. 
Astfel la inceput, obiectele vin cu viteza redusa si o linie completata valoreaza 
50 de puncte. Dupa ce utilizatorul atinge o bariera de puncte specifica nivelului, trece la nivelul urmator si creste viteza
si punctele corespondente unui rand completat.
Jocul se termina cand utilizatorul nu mai are vieti ramase.

### Hardware folosit
<ul>
  <li>placa arduino</li>
  <li>2-3 breadboard-uri</li>
  <li>jumper wires</li>
  <li>condensatori</li>
  <li>joystick pentru control</li>
  <li>display pentru afisarea scorului</li>
  <li>matrice led max7219</li>
</ul>

#### Detalii Software
Obiectele Tetris sunt generate random folosind Design Pattern-ul Builder, o clasa Singleton care instantiaza obiectele respective.
