-- Define Achievements + their conditions

achievement_add("Baby steps", "OK, you opened the game. What now?","default")

achievement_add("Raise your hand", "Lift the hand in the air like you don't care...","default")
achievement_setProperty("maxHandY", 0)
achievement_condition("Raise your hand", "maxHandY",">=", 5)

achievement_add("Make it rain", "Water for everyone!","default")
achievement_setProperty("maxWaterFallingHeight", 0)
achievement_condition("Make it rain", "maxWaterFallingHeight", ">=", 15)

achievement_add("Rain... Rain everywhere", "Damn... Forgot my umbrella","default")
achievement_setProperty("rainStrength", 0)
achievement_condition("Rain... Rain everywhere", "rainStrength", ">", 0)

achievement_add("Rock solid!", "Lava + Water = Rock","default")
achievement_setProperty("bedrock", 0)
achievement_condition("Rock solid!", "bedrock", "==", 1)

achievement_add("Let clouds arise!", "Foggy... isn't it?!", "default")
achievement_setProperty("steam", 0)
achievement_condition("Let clouds arise!", "steam", ">=", 3)
