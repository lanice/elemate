
achievement_add("Baby steps", "OK, you opened the game. What now?","default");
achievement_unlock("Baby steps");

achievement_add("Raise your hand", "Lift the hand in the air like you don't care...","default");
achievement_condition("Raise your hand", "maxHandY",">=",15)
achievement_setProperty("maxHandY", 0);

achievement_add("Make it rain", "Water for everyone!","default");
