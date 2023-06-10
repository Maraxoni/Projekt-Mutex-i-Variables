compilemutex: ProjektMutex.c
	gcc ProjektMutex.c -o ProjektMutex -lssl -lcrypto
compilevariables: ProjektVariables.c
	gcc ProjektVariables.c -o ProjektVariables -lssl -lcrypto
