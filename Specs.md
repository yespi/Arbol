#Al iniciar por primera vez:

1.	Generar deseos al azar, basado en un parámetro de máximo número azar.
2.	Especificar para cada deseo al azar, un tiempo de vida al azar, para que no mueran todos por igual.

#####Grupos de leds (Deseo):

1.	Todos los leds de un grupo (deseo) se apagan por igual.
2.	Apagado mediante FADE.
3.	Tiempo máximo de vida parametrizable.
4.	Cuando se recibe un nuevo deseo:
    - Todo el árbol parpadea con el color elegido del deseo.
    - Se recupera es estado de todos los deseos. El árbol vuelve al estado antes de recibir el deseo nuevo.
    - Se busca un hueco/grupo libre al azar.
    - Se muestra el deseo (grupo de leds) parpadeando en la posición que le ha tocado con el color elegido.

####General:

1.	El árbol tiene que tener un mínimo de deseos siempre activos. Parámetro.
    - Si no se reciben deseos hay que generar alguno de forma aleatoria: tiempo, color y posición.
    - Los deseos al azar deben estar basados en el parámetro de máximo número azar.
2.	Si pasado un tiempo (parámetro), no recibe ningún deseo nuevo, ejecutar un efecto de reclamo.

