/* Wish ohjelma 
Tekijä: Marianne Seppänen
Pvm: 13.7.2021
Ohjelma toimii kuten shell, eli sille voi syöttää käskyjä, joita se toteuttaa. 
exit, cd ja path-käskyt on toteutettu erikseen. 
Koodaamisessa on käytetty apuna C-ohjelmoinnin perusteiden ohjelmointiopasta, "C-kieli ja käytännön ohjelmointi, osa 1"
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char error_message[30] = "An error has occurred\n";

void program(char command[]);

/* Aliohjelma, johon tiedoston sisältö tai käyttäjän syöttämät käskyt viedään */
void program(char command[]) {

	char cdcomm[] = "cd";
	char pathcomm[] = "path";
	char searchpath[30] = "/bin";

	/* Parsitaan käsky, otetaan käskyn osat talteen */
	char *found = strtok(command," ");
	char first[20];
	char* arlist[10] = {};
	int i=0;
	memcpy(first, found, 20); /* Kopioidaan käskyn eka osa first-muuttujaan */
	found = strtok(NULL," ");
	while (found) { /* Käydään loput käskystä läpi, kasataan osat arlist-arrayhin */ 
			arlist[i] = found;
			i++;
			found = strtok(NULL," ");
			}

	/* Testataan, onko käyttäjän antama käsky cd tai path, jos on niin toteutetaan ne */
	if (strcmp(first,cdcomm) == 0) {
		//fprintf(stdout, "%s %s will be executed \n", command, arlist[0]);
		int s = chdir(arlist[0]);
		if (s != 0) { write(STDERR_FILENO, error_message, strlen(error_message)); } }
		
	else if (strcmp(first,pathcomm) == 0) {
		// fprintf(stdout, "%s will be executed \n", command); 
		strcpy(searchpath, arlist[0]);
		// fprintf(stdout, "path: %s \n", searchpath);
		 } 
		

	/* Luodaan fork()-funktiolla lapsiprosessi, jossa käsky ajetaan execv()-funktiolla. Vanhempi odottaa lapsen suoritusta wait()-funktiolla.
	Apuna käytetty kurssimateriaaleja, https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf */
	else {
		int rc = fork();
		if (rc < 0) { /* fork epäonnistui */
			write(STDERR_FILENO, error_message, strlen(error_message));
			exit(1);

		} else if (rc == 0) { /* lapsi, eli uusi prosessi */
			//fprintf(stdout, "Entering child process\n");
			strcat(searchpath, "/");
			strcat(searchpath, first);  

			/* Tarkistetaan, onko polku toimiva access()-systeemikutsun avulla, jos on niin aloitetaan toiminto execv()-käskyllä */
			if (access(searchpath, X_OK)==0) {
				execv(searchpath, arlist); }
			else { 
				write(STDERR_FILENO, error_message, strlen(error_message)); }

			
		} else { /* vanhempi, eli käynnissä oleva prosessi odottaa */
			wait(NULL);
			//fprintf(stdout, "Back to the parent process\n");
		}
	}
}

int main(int argc, char *argv[]) {

char command[80]; 
char ending[] = "exit";

/* Ei argumentteja */
if (argc == 1) {
	/* While-loop, joka kysyy käyttäjältä aina uutta käskyä */
	while (1==1) {	
		fprintf(stdout, "wish> ");
		fgets(command, 79, stdin); 
		command[strcspn(command, "\n")] = 0;

		/* Testataan, onko annettu käsky exit, jos on niin lopetetaan */
		if (strcmp(command,ending) == 0) {
			exit(1); 
		}

		/* Lähetetään käsky aliohjelmaan */
		program(command);
	
	}	
}

/* Argumenttina 1 tiedosto */
else if (argc == 2) {
	/* Avataan tiedosto, jossa käskyt ovat */
	char muisti[100]; 
	FILE *tiedosto;
	if ((tiedosto = fopen(argv[1], "r")) != NULL) { 
		while (fgets(muisti, 99, tiedosto) != NULL) {
			//fprintf(stdout, "Tiedostossa: %s \n", muisti); 
			/* Lähetetään tiedoston sisältö rivi kerrallaan aliohjelmaan, jossa käskyt toteutetaan */
			muisti[strcspn(muisti, "\n")] = 0;
			program(muisti);
		} 
	} else { /* Virheenkäsittely, jos tiedosto ei aukea */
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1); }
	fclose(tiedosto);

}

/* Jos argumentteja joku muu määrä, ohjelma vain palauttaa 0 ja loppuu */
return(0);
}
