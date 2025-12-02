    
    Giro suave (SOFT): se reduce la velocidad del motor del lado de la línea y se mantiene/incrementa la del lado opuesto. Resultado: curva amplia.
    Giro con motor interior detenido (STOP): el motor del lado de la línea se pone a 0 y el otro motor avanza: giro más pronunciado.
    Giro fuerte (REVERSE): el motor del lado de la línea invierte su giro mientras el otro avanza: giro en el lugar o muy agresivo.

    Cambia el modo de giro descomentando la directiva correspondiente al inicio del archivo: TURN_MODE_SOFT, TURN_MODE_STOP o TURN_MODE_REVERSE.
    Si el robot gira en sentido contrario al esperado, invierte las señales digitales (LOW/HIGH) en las funciones moverAdelante / girar* para el motor correspondiente.
    Ajusta V_GIRO_SUAVE / V_GIRO_FUERTE y V_BUSCAR para que la respuesta física coincida con lo que quieres.
    Si prefieres que "si detecta blanco en el izquierdo accione solo el derecho" -> usa TURN_MODE_STOP (detiene motor interior y mueve el exterior).
