#define N_THREADS 3
#define N_MV 10
#define N_STR 48
#define N_THREADS_OUT 10

struct vt {
char identificador[37];
int assembleia;
char mesa_voto;
long int marca_tempo_entrada;
long int marca_tempo_saida;
int item_voto;
};

char *mv[N_MV] =
 {
	"A","B","C","D",
	"E","F","G","H",
	"I","J"
};

