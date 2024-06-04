#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <sys/select.h>

#define FOREVER 	    1
#define INIT_ROW 	    20
#define INIT_COL 	    20
#define SELECT_EVENT	1
#define SELECT_NO_EVENT 0
#define S_TO_WAIT 		10
#define MILIS_TO_WAIT   0
#define KEYBOARD		0
#define COLOR_PINK      8
#define COLOR_ORANGE    9
#define COLOR_BROWN     10
#define COLOR_PURPLE    11
#define COLOR_YELLOW2   12
#define COLOR_GREY 		13

int **new_game_resume(int **joc, int *score, int opt, int *end, int *exit, int *win);
void menu(int **joc, int *score, int ok, int *end, int *exit, int *win);

/* am creat 5 culori suplimentare */
void initialize_color() {
    start_color();
    init_color(COLOR_PINK, 996, 199, 597);
    init_color(COLOR_ORANGE, 996, 500, 0);
    init_color(COLOR_BROWN, 398, 199, 0);
    init_color(COLOR_PURPLE, 398, 0, 398);
    init_color(COLOR_YELLOW2, 996, 996, 196);
	init_color(COLOR_GREY, 500, 500, 500);
    init_pair(5, COLOR_GREEN, COLOR_WHITE); // culoare highlight meniu
	init_pair(1, COLOR_WHITE, COLOR_BLACK); // culoare standard ecran
	init_pair(3, COLOR_BLACK, COLOR_GREY); // culoare default celula
    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(4, COLOR_WHITE, COLOR_BLUE);
    init_pair(8, COLOR_WHITE, COLOR_GREEN);
    init_pair(16, COLOR_WHITE, COLOR_CYAN);
    init_pair(32, COLOR_BLACK, COLOR_YELLOW);
    init_pair(64, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(128, COLOR_WHITE, COLOR_PINK);
    init_pair(256, COLOR_WHITE, COLOR_ORANGE);
    init_pair(512, COLOR_WHITE, COLOR_BROWN);
    init_pair(1024, COLOR_WHITE, COLOR_PURPLE);
    init_pair(2048, COLOR_BLACK, COLOR_YELLOW2);
}

/* Transforma un numar in sir de caractere
Folosita pentru a putea afisa numerele generate aleatoriu pe ecran */
char * noSTR(int a) {
	int k = 0, i;
	char *s;
	s = (char *) malloc (5 * sizeof (char));
	while(a) {
		for (i = k; i > 0; i--)
			s[i] = s[i - 1];
		s[0] = (a % 10) + '0';
		k++;
		a /= 10;
	}
	s[k] = '\0';
	return s;
}

/* Verifica daca matricea joc nu are locuri libere */
int is_full(int **joc) {
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (joc[i][j] == 0)
				return 0;
	return 1;
}

/* Verifica daca matricea joc e complet goala */
int is_empty(int **joc) {
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (joc[i][j] == 1)
				return 0;
	return 1;
}
/* Verifica daca jocul e castigat */
int is_winner(int **joc) {
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if(joc[i][j] == 2048)
				return 1;
	return 0;
}
/* Verifica daca mutarea a fost valida,
adica daca s-a modificat tabla de joc */
int invalid (int **joc, int **joc1) {
	int i, j;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			if (joc[i][j] != joc1[i][j])
				return 0;
	return 1;
}

/* Verifica in matricea joc daca mai sunt mutari valide
O mutare valida presupune cel putin doua elemente identice,
situate pe aceeasi linie / coloana */
int valid_moves(int **joc) {
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			int x = joc[i][j];
			/* considera separat cazurile elementelor
			de pe primul, respectiv ultimul rand
			sau coloana */
			if (i == 0){
				if (j == 0){
					if (joc[i][j + 1] == x || joc[i + 1][j] == x)
						return 1;
				} 
				else if (j == 3){
					if (joc[i][j - 1] == x || joc[i + 1][j] == x)
						return 1;
				}
				else if (0 < j && j < 3){
					if (joc[i][j - 1] == x || joc[i][j + 1] == x || joc[i + 1][j] == x)
						return 1;
				}
			}
			else if (i == 3) {
				if (j == 0){
					if (joc[i][j + 1] == x || joc[i - 1][j] == x)
						return 1;
				} 
				else if (j == 3){
					if (joc[i][j - 1] == x || joc[i - 1][j] == x)
						return 1;
				}
				else if (0 < j && j < 3){
					if (joc[i][j - 1] == x || joc[i][j + 1] == x || joc[i - 1][j] == x)
						return 1;
				}
			}
			else {
				if (j == 0){
					if (joc[i][j + 1] == x || joc[i + 1][j] == x || joc[i - 1][j] == x)
						return 1;
				}
				else if (j == 3){
					if (joc[i][j - 1] == x || joc[i + 1][j] == x || joc[i - 1][j] == x)
						return 1;
				}
				else if (0 < j && j < 3){
					if (joc[i][j - 1] == x || joc[i][j + 1] == x || joc[i + 1][j] == x || joc[i - 1][j] == x)
						return 1;
				}
			}
		}
	}
	return 0;
}

/* Genereaza aleatoriu doi indici pt matrice, 
cu mentiunea ca acestia trebuie sa desemneze o pozitie libera*/
void random_pos(int *i, int *j, int **joc) {
	do{
		srand(time(NULL));
		*i = rand() % 4;
		*j = rand() % 4;
	}while (joc[*i][*j] != 0);
}

/* Genereaza aleatoriu unul dintre numerele 2 sau 4 */
int random_no(){
	int a = rand() % 2;
	return a * 2 + 2;
}

/* Marcheaza numarul in matricea joc si il afiseaza pe ecran */
int ** mark (int **joc, int i, int j, int a, WINDOW *wnd, int x, int y){
    initialize_color();
	int dx = x / 4;
	int dy = y / 4;
	joc[i][j] = a;
	char *s = noSTR (a);
	if (a){
		/* sterge numarul anterior si il adauga pe cel nou */
		mvwaddstr(wnd, dy / 2, dx / 2, "     ");
		mvwaddstr(wnd, dy / 2, dx / 2, s);
		/* seteaza culoarea de fundal a ferestrei */
        wbkgd(wnd, COLOR_PAIR(a));
	}
	else {
		mvwaddstr(wnd, dy / 2, dx / 2, "     ");
        wbkgd(wnd, COLOR_PAIR(3));
    }
	wrefresh (wnd);
	return joc;
}

/* Returneaza data si ora curenta */
char * date () {
	time_t time1;
	struct tm *time2;
	time(&time1);
	time2 = localtime (&time1);
	char *s;
	s = (char *) malloc (80 * sizeof (char));
	/* afiseaza data dupa formatul dd.mm.yy hour:min */
	strftime (s, 80, "%e.%m.%G %R", time2);
	return s;
}

/* Interschimba elementele de pe o coloana a.i. 0 sa fie la final */
int **Push0ToEnd (int **joc, int k) {
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = i + 1; j < 4; j++)
			if (joc[i][k] == 0 && joc[j][k]) {
				int aux = joc[i][k];
				joc[i][k] = joc[j][k];
				joc[j][k] = aux;
			}
	return joc;
}

/* Interschimba elementele de pe o coloana a.i. 0 sa fie la inceput */
int **Push0ToBeg (int **joc, int k) {
	int i, j;
	for (i = 3; i >= 1; i--)
		for (j = i - 1; j >= 0; j--)
			if (joc[i][k] == 0 && joc[j][k]) {
				int aux = joc[i][k];
				joc[i][k] = joc[j][k];
				joc[j][k] = aux;
			}
	return joc;
}

/* Interschimba elementele de pe o linie a.i. 0 sa fie in stanga (la inceput) */
int **Push0ToLeft (int **joc, int k) {
	int i, j;
	for (i = 3; i >= 1; i--)
		for (j = i - 1; j >= 0; j--)
			if (joc[k][i] == 0 && joc[k][j]) {
				int aux = joc[k][i];
				joc[k][i] = joc[k][j];
				joc[k][j] = aux;
			}
	return joc;
}

/* Interschimba elementele de pe o linie a.i. 0 sa fie in dreapta (la final) */
int **Push0ToRight (int **joc, int k) {
	int i, j;
	for (i = 0; i < 3; i++)
		for (j = i + 1; j < 4; j++)
			if (joc[k][i] == 0 && joc[k][j]) {
				int aux = joc[k][i];
				joc[k][i] = joc[k][j];
				joc[k][j] = aux;
			}
	return joc;
}

/* Functionarea propriu-zisa a jocului */
int **game (int **joc, char c, int *score) {
	int i, j;
	if (tolower (c) == 'w') {
		for (i = 0; i < 4; i++) {
			/* muta elementele nule in partea de jos a coloanei */
			joc = Push0ToEnd (joc, i);
			for (j = 0; j < 3; j++) 
			/* uneste celulele identice de pe coloana i si modifica matricea */
				if (joc[j][i] == joc[j + 1][i] && joc[j][i]) {
					joc[j][i] *= 2;
					(*score) += joc[j][i];
					joc[j + 1][i] = 0;
				}
			/* raman elemente nule din cauza unirii celulelor identice,
			deci trebuie sa le mut din nou in partea de jos*/
			joc = Push0ToEnd (joc, i);
		}
	}
	else if (tolower (c) == 's') {
		for (i = 0; i < 4; i++) {
			/* muta elementele nule in partea de sus a coloanei */
			joc = Push0ToBeg (joc, i);
			for (j = 3; j >= 1; j--)
			/* uneste celulele identice de pe coloana i si modifica matricea */
				if (joc[j][i] == joc[j - 1][i] && joc[j][i]) {
					joc[j][i] *= 2;
					(*score) += joc[j][i];
					joc[j - 1][i] = 0;
				}
			joc = Push0ToBeg (joc, i);
		}
	}
	else if (tolower (c) == 'd') {
		for (i = 0; i < 4; i++) {
			/* muta elementele nule in stanga */
			joc = Push0ToLeft (joc, i);
			for (j = 3; j >= 1; j--)
			/* uneste celulele identice de pe linia i si modifica matricea */
				if (joc[i][j] == joc[i][j - 1] && joc[i][j]) {
					joc[i][j] *= 2;
					(*score) += joc[i][j];
					joc[i][j - 1] = 0;
				}
			joc = Push0ToLeft (joc, i);
		}
	}
	else if (tolower(c) == 'a'){
		for (i = 0; i < 4; i++) {
			/* muta elementele nule in dreapta */
			joc = Push0ToRight (joc, i);
			for (j = 0; j < 3; j++)
			/* uneste celulele identice de pe linia i si modifica matricea */
				if (joc[i][j] == joc[i][j + 1] && joc[i][j]) {
					joc[i][j] *= 2;
					(*score) += joc[i][j];
					joc[i][j + 1] = 0;
				}
			joc = Push0ToRight (joc, i);
		}
	}
	return joc;
}
/* determina mutarea care elibereaza cele mai multe celule */
int ** best_move (int **joc, int *score) {
	/* calculeaza numarul maxim de celule nule */
	int max = 0;
	int i, j, k, **joc1, score_max, **joc_max, scor;
	char s[] = "wasd";
	joc1 = (int **) malloc(4 * sizeof (int *));
	joc_max = (int **) malloc(4 * sizeof (int *));
	for (i = 0; i < 4; i++){
		joc1[i] = (int *) malloc(4 * sizeof (int));
		joc_max[i] = (int *) malloc(4 * sizeof (int));
	}
	/* pt fiecare dintre cele 4 mutari posibile,
	calculeaza numarul de celule nule care rezulta */
	for (k = 0; k < 4; k++) {
		int nr = 0; // numar de celule nule
		for (i = 0; i < 4; i++)
			for (j = 0; j < 4; j++)
				joc1[i][j] = joc[i][j];
		joc1 = game (joc1, s[k], &scor);
		for (i = 0; i < 4; i++)
			for (j = 0; j < 4; j++)
				if (joc1[i][j] == 0)
					nr++;
		if (max < nr){
			max = nr;
			/* retine tabla modificata,
			corespunzatoare cele mai bune mutari */
			for (i = 0; i < 4; i++)
				for (j = 0; j < 4; j++)
					joc_max[i][j] = joc1[i][j];
			score_max = scor;
		}
	}
	*score += score_max;
	return joc_max;
}

/* Afiseaza legenda comenzilor valide */
void afisare_legenda (int **joc, int dim, int ncols) {
	int i, j, k, **joc1, scor = 0, n = 0;
	char s[] = "WASD";
	joc1 = (int **) malloc(4 * sizeof (int *));
	for (i = 0; i < 4; i++)
		joc1[i] = (int *) malloc(4 * sizeof (int));
	for (k = 0; k < 4; k++) {
		/* Am facut o copie a tablei,
		pentru testarea variantelor de apasare a tastelor*/
		for (i = 0; i < 4; i++)
			for (j = 0; j < 4; j++)
				joc1[i][j] = joc[i][j];
		game(joc1, s[k], &scor);
		/* Mutare valida, afisare corespunzatoare */
		if (invalid (joc, joc1) == 0) {
			/* Variabila n numara randurile pt afisare */
			n++;
			if (k == 0)
				mvwaddstr (stdscr, dim / 4 + n + 1, 3 * ncols / 4, "W - UP");
			else if (k == 1)
				mvwaddstr (stdscr, dim / 4 + n + 1, 3 * ncols / 4, "A - LEFT");
			else if (k == 2)
				mvwaddstr (stdscr, dim / 4 + n + 1, 3 * ncols / 4, "S - DOWN");
			else
				mvwaddstr (stdscr, dim / 4 + n + 1, 3 * ncols / 4, "D - RIGHT");
		}
	}
}
/* Goleste liniile in care afisez legenda de comenzi */
void clear_legenda(int dim, int ncols){
	int i;
	for (i = 0; i < 4; i++)
		mvwaddstr (stdscr, dim / 4 + i + 2, 3 * ncols / 4, "         ");
}

/* Functia meniu */
void menu (int **joc, int *score, int ok, int *end, int *exit, int *win) {
	/* variabila ok indica daca e prima intrare in meniu,
	sau se revine in meniu dintr-un joc anterior, 
	caz in care ok = 1 */
    int row, col, new_row, new_col, dim;
	int nrows, ncols;
	int i, j;
	char c;
	if (ok == 0)
		initscr();
	getmaxyx(stdscr, nrows, ncols);
	clear();
	noecho();
	cbreak();
	curs_set(0);
    initialize_color();
	/* calcul coordonate x, y pentru tabla curenta,
	a.i. textul sa fie centrat*/
	if (nrows < ncols)
		dim = nrows;
	else
		dim = ncols;
	int y = 3 * dim / 4;
	int x = 3 * dim / 2;
	int dim_y = (nrows - y) / 2 + 1;
	int dim_x = (ncols - x) / 2 + 1;
	
	mvaddstr(dim_y, dim_x, "2048 - MENU");
    mvaddstr(dim_y + 1, dim_x - 5, "New Game");
	if (ok == 0)
    	mvaddstr(dim_y + 2, dim_x - 5, "Quit");
	else{
		mvaddstr(dim_y + 2, dim_x - 5, "Resume");
		mvaddstr(dim_y + 3, dim_x - 5, "Quit");
	}
    row = dim_y + 1;
    col = dim_x - 5;
	new_row = row;
	new_col = col;

	while (FOREVER) {
		move(row, col);
		/* highlight pe optiunea curenta */
		mvchgat(row, col, -1, A_STANDOUT, 5, NULL);
		refresh();

		c = getchar();

		if (tolower(c) == 'w') {
			if (row > 0)
				new_row = row - 1;
			new_col = col;
		}
		else if (tolower(c) == 's') {
			if (row + 1 < nrows)
				new_row = row + 1;
			else
				new_row = 1;
			new_col = col;
		}
		/* iesire din joc */
		else if (tolower(c) == 'q'){
			*exit = 1;
			break;
		}
		if (c == '\r') {
			/* s-a apasat Enter pe optiunea de new_game */
			if (new_row == dim_y + 1){
				*score = 0;
				for (i = 0; i < 4; i++)
					for (j = 0; j < 4; j++)
						joc[i][j] = 0;
				new_game_resume(joc, score, 1, end, exit, win);
				break;
			}
			/* s-a apasat Enter pe optiunea de resume */
			if (new_row == dim_y + 2 && ok == 1){
				new_game_resume(joc, score, 2, end, exit, win);
				break;
			}
			/* s-a apasat Enter pe optiunea de quit,
			ok = 0 imi indica faptul ca opțiunea de quit se afla
			pe a doua linie (nu exista resume) */
			if (new_row == dim_y + 3 || (new_row == dim_y + 2 && ok == 0)){
				*exit = 1;
				break;
			}
		}
		/* se revine la text normal */
		mvchgat(row, col, -1, A_NORMAL, 1, NULL);
		row = new_row;
		col = new_col;
		move(row, col);
		mvchgat(row, col, -1, A_STANDOUT, 5, NULL);
		refresh();
	}
}

/* Functia pentru comanda New Game si pt Resume*/
int ** new_game_resume(int **joc, int *score, int opt, int *end, int *exit, int *win) {
	/* initializarea variabilelor pt folosirea lui select */
	fd_set read_descriptors;
	struct timeval timeout;
	int nfds = 1, sel;
	FD_ZERO(&read_descriptors);
	FD_SET(KEYBOARD, &read_descriptors);
	timeout.tv_sec = S_TO_WAIT;
	timeout.tv_usec = MILIS_TO_WAIT;

	int nrows, ncols, dim, i, j, a, b, dim_x, dim_y;
	int **joc1;
	joc1 = (int **) malloc (4 * sizeof (int *));
	for (i = 0; i < 4; i++)
		joc1[i] = (int *) malloc (4 * sizeof (int));
	char c;
	clear();
	refresh();
    initialize_color();
	getmaxyx(stdscr, nrows, ncols);
	/* calcul coordonate x, y pentru tabla curenta,
	a.i. textul si tabla sa fie centrate */
	if (nrows < ncols)
		dim = nrows;
	else
		dim = ncols;
	int y = 3 * dim / 4;
	int x = 3 * dim / 2;
    int dx = x / 4;
    int dy = y / 4;
	/* Am creat o fereastra pentru fiecare celula,
	si am initializat tabla de joc */
    WINDOW *wnd[4][4];
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) {
			/* coordonate pentru planul tablei */
			dim_y = (nrows - y) / 2 + dy * i + 1;
			dim_x = (ncols - x) / 2 + dx * j + 1;
            wnd[i][j] = newwin (dy, dx, dim_y, dim_x);
            wclear (wnd[i][j]);
            mvwaddstr(wnd[i][j], dy / 2, dx / 2, "     ");
			/* initializare background fereastra cu culoarea default */
            wbkgd(wnd[i][j], COLOR_PAIR(3));
            wrefresh(wnd[i][j]);
        }
    i = j = 0;
	/* Daca opt = 1, inseamna ca am selectat new game, 
	altfel resume */
	if (opt == 1){
		/* se genereaza doua numere random in celule random */
		random_pos(&i, &j, joc);
		a = random_no();
		joc = mark (joc, i, j, a, wnd[i][j], x, y);
		random_pos(&i, &j, joc);
		b = random_no();
		joc = mark (joc, i, j, b, wnd[i][j], x, y);
	}
	else{
		/* pt resume,
		se afiseaza tabla de joc ramasa de la ultimul joc */
		for (i = 0; i < 4; i++)
			for (j = 0; j < 4; j++)
				joc = mark (joc, i, j, joc[i][j], wnd[i][j], x, y);
	}
	mvwaddstr (stdscr, dim / 4, 3 * ncols / 4, "Date: ");
	mvwaddstr (stdscr, dim / 4 + 1, 3 * ncols / 4, "Score: ");
	refresh();

	dim_y = (nrows - y) / 2 + 1;
	dim_x = (ncols - x) / 2 + 1;

	while(FOREVER) {
		char *s = date();
		mvwaddstr (stdscr, dim / 4, 3 * ncols / 4 + 8, s);
		s = noSTR(*score);
		mvwaddstr (stdscr, dim / 4 + 1, 3 * ncols / 4 + 8, s);
		mvwaddstr (stdscr, 1, dim_x, "You have 10 seconds to press a key!");
		clear_legenda(dim, ncols);
		afisare_legenda(joc, dim, ncols);
		refresh();
		sel = select(nfds, &read_descriptors, NULL, NULL, &timeout);
		if (sel == SELECT_EVENT) {
			if ((c = getch ()) != ERR) {
				/* goleste buffer-ul (ramane Enter) */
				flushinp();
				if (tolower(c) == 'q'){
					menu(joc, score, 1, end, exit, win);
					break;
				}
				else if (!strchr ("wsad", tolower (c))) {
					mvwaddstr (stdscr, nrows - 2, dim_x, "Press one of the valid keys! (W / A / S / D)");
					refresh();
				}
				else {
					/* se sterge textul de eroare (pt invalid key / move) */
					mvwaddstr (stdscr, nrows - 2, dim_x, "                                            ");
					/* creez o copie a tablei de joc pt verificare ulterioara */
					for (i = 0; i < 4; i++)
						for (j = 0; j < 4; j++)
							joc1[i][j] = joc[i][j];
					joc = game (joc, c, score);
					/* cazul unei mutari valide */
					if (invalid (joc, joc1) == 0)
					{
						/* se printeaza noua tabla de joc si un nou numar random*/
						for (i = 0; i < 4; i++)
							for (j = 0; j < 4; j++)
								joc = mark (joc, i, j, joc[i][j], wnd[i][j], x, y);
						random_pos(&i, &j, joc);
						a = random_no();
						joc = mark (joc, i, j, a, wnd[i][j], x, y);
						wrefresh (wnd[i][j]);
						refresh();
						/* se verifica daca jocul e castigat */
						if (is_winner(joc)) {
							*end = 1;
							*win = 1;
							break;
						}
						/* se verifica daca nu mai sunt mutari valide */
						if (is_full (joc) && !valid_moves (joc)){
							*end = 1;
							break;
						}
					}
					else {
						mvwaddstr (stdscr, nrows - 2, dim_x, "Not a valid move, try again.");
						refresh();
					}
				}
			}
			flushinp();
		}
		/* user-ul nu a apasat nicio tasta timp de 10 secunde */
		else if (sel == SELECT_NO_EVENT){
			mvwaddstr (stdscr, 1, dim_x, "                                   ");
			mvwaddstr (stdscr, 1, dim_x, "Time's up!");
			refresh();
			/* se printeaza cea mai buna mutare existenta si un numar random */
			joc = best_move(joc, score);
			for (i = 0; i < 4; i++)
				for (j = 0; j < 4; j++)
					joc = mark (joc, i, j, joc[i][j], wnd[i][j], x, y);
			random_pos(&i, &j, joc);
			a = random_no();
			joc = mark (joc, i, j, a, wnd[i][j], x, y);
			wrefresh (wnd[i][j]);
			if (is_winner(joc)) {
				*end = 1;
				*win = 1;
				break;
			}
			if (is_full (joc) && !valid_moves (joc)){
				*end = 1;
				break;
			}
			mvwaddstr (stdscr, 1, dim_x, "          ");
			refresh();
		}
		FD_SET(KEYBOARD, &read_descriptors);
		
		timeout.tv_sec = S_TO_WAIT;
		timeout.tv_usec = MILIS_TO_WAIT;
	}
	return joc;
}

int main(void)
{
	/* variabila end retine daca s-a terminat jocul (nicio mutare valida) */
	/* variabila exit retine daca s-a iesit din joc cu quit */
	/* variabila win retine daca s-a castigat jocul */
	int **joc, score = 0, i, j, end = 0, exit = 0, win = 0;
	int nrows, ncols;
	joc = (int **) malloc(4 * sizeof(int *));
	for (i = 0; i < 4; i++){
		joc[i] = (int *) malloc(4 * sizeof(int));
		for (j = 0; j < 4; j++)
			joc[i][j] = 0;
	}
	do{
		menu(joc, &score, 0, &end, &exit, &win);
		/* afisare ecran de final joc */
		if (end == 1){
			clear();
			getmaxyx(stdscr, nrows, ncols);
			char *s = noSTR(score);
			if (win == 0)
				mvwaddstr(stdscr, nrows / 2, ncols / 2, "GAME OVER !");
			else
				mvwaddstr(stdscr, nrows / 2, ncols / 2, "YOU WON !");

			mvwaddstr(stdscr, nrows / 2 + 1, ncols / 2, "SCORE: ");
			mvwaddstr(stdscr, nrows / 2 + 1, ncols / 2 + 8, s);
			mvwaddstr(stdscr, nrows / 2 + 2, ncols / 2, "PLAY AGAIN? (Y / N)");
			refresh();
			char c;
			/* asteapta un raspuns (y/n) de la user */
			do{
				c = getchar ();
				/* se iese din joc */
				if (tolower (c) == 'n')
					exit = 1;
			}while(tolower (c) != 'y' && tolower (c) != 'n');
			/* se reinitializeaza variabilele */
			score = 0;
			for (i = 0; i < 4; i++)
				for (j = 0; j < 4; j++)
					joc[i][j] = 0;
			end = 0;
			win = 0;
		}
	/* continua cat timp user-ul nu iese din joc */
	}while(exit == 0);
	endwin();
	return 0;
}