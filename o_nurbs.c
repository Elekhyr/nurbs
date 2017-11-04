#include <GL/gl.h>
#include <math.h>
#include "o_objet.h"
#include "u_table.h"
#include "t_geometrie.h"

struct nurbs
{
  Table_quadruplet table_nurbs;
  int nb_pts;
  int degre;
  double nouveau_noeud;
  Table_triplet affichage;
  Table_flottant sequence_nodale;
  Booleen polygone_ctrl;
} ; 

static int cacluler_r(Table_flottant *U, int n, int k, double u)
{
	
	int i  = 0;
	for (i = k-1; i < n ; i++) {
		if (U->table[i] <= u && u < U->table[i+1]) {
			return i;
		}
	}
	return n-1;
}

static Triplet calcPoint(Table_quadruplet pts_nurbs, Table_flottant sequence_nodale, double u, int r, int k)
{
	Table_quadruplet T;
	T.nb = pts_nurbs.nb;
	ALLOUER(T.table, pts_nurbs.nb);
	Triplet P;

	for(int i=0 ; i < pts_nurbs.nb ; i++) {
		//coord homogenes
		T.table[i].x  = pts_nurbs.table[i].x * pts_nurbs.table[i].h;
		T.table[i].y  = pts_nurbs.table[i].y * pts_nurbs.table[i].h;
		T.table[i].z  = pts_nurbs.table[i].z * pts_nurbs.table[i].h;
		T.table[i].h  = pts_nurbs.table[i].h;
	}

	for(int j = 1; j <= k - 1 ; j++) {
		for(int i = r ; i >= r-k+1+j; i--) {
			
			T.table[i].x = T.table[i].x * 
						(u - sequence_nodale.table[i])/
						(sequence_nodale.table[i+k-j] - sequence_nodale.table[i]) +
						T.table[i-1].x * 
						(sequence_nodale.table[i+k-j] - u)/
						(sequence_nodale.table[i+k-j] - sequence_nodale.table[i]);
						
			T.table[i].y = T.table[i].y * 
						(u - sequence_nodale.table[i])/
						(sequence_nodale.table[i+k-j] - sequence_nodale.table[i]) +
						T.table[i-1].y * 
						(sequence_nodale.table[i+k-j] - u)/
						(sequence_nodale.table[i+k-j] - sequence_nodale.table[i]);
						
			T.table[i].z = T.table[i].z * 
						(u - sequence_nodale.table[i])/
						(sequence_nodale.table[i+k-j] - sequence_nodale.table[i]) +
						T.table[i-1].z * 
						(sequence_nodale.table[i+k-j] - u)/
						(sequence_nodale.table[i+k-j] - sequence_nodale.table[i]);
						
			T.table[i].h = T.table[i].h * 
						(u - sequence_nodale.table[i])/
						(sequence_nodale.table[i+k-j] - sequence_nodale.table[i]) +
						T.table[i-1].h * 
						(sequence_nodale.table[i+k-j] - u)/
						(sequence_nodale.table[i+k-j] - sequence_nodale.table[i]);
		}
	}
	//~ //projection en cartesien
	
	T.table[r].x  = T.table[r].x / T.table[r].h;
	T.table[r].y  = T.table[r].y / T.table[r].h;
	T.table[r].z  = T.table[r].z / T.table[r].h;
	
	P.x = T.table[r].x;
	P.y = T.table[r].y;
	P.z = T.table[r].z;
	free(T.table);
	return P;
}

static void affiche_nurbs(struct nurbs *o)
{
	glBegin(GL_LINE_STRIP) ;
	
	for(int k=0 ; k < o->nb_pts ; k++)
	{
		glVertex3f(o->affichage.table[k].x, o->affichage.table[k].y, o->affichage.table[k].z);
	}
	
	glEnd();

	//affichage polynome controle
	if (o->polygone_ctrl) {
		int j;
		
		glBegin(GL_LINE_STRIP) ;
		
		for(j=0  ; j<o->table_nurbs.nb ; j++)
			glVertex3f(
				o->table_nurbs.table[j].x,
				o->table_nurbs.table[j].y,
				o->table_nurbs.table[j].z); 

		glEnd();
	}
}

static void inserer_noeud(Table_quadruplet *table_nurbs, int* degre, Table_flottant *sequence_nodale, double nouveau_noeud)
{//todo
	Table_flottant nouvelle_sequence_nodale;
	nouvelle_sequence_nodale.nb = sequence_nodale->nb + 1;
	ALLOUER(nouvelle_sequence_nodale.table, nouvelle_sequence_nodale.nb);
	
	unsigned i;
	int inserted = 0;
	for(i = 0; i < sequence_nodale->nb; ++i)
	{
		if (inserted == 0)
		{
			if (sequence_nodale->table[i] < nouveau_noeud)
				nouvelle_sequence_nodale.table[i] = sequence_nodale->table[i];
			else
			{
				nouvelle_sequence_nodale.table[i] = nouveau_noeud;
				nouvelle_sequence_nodale.table[i + 1] = sequence_nodale->table[i];
				inserted = 1;
			}
		}
		else
			nouvelle_sequence_nodale.table[i + 1] = sequence_nodale->table[i];
	}
	if (inserted == 0)
		nouvelle_sequence_nodale.table[nouvelle_sequence_nodale.nb - 1] = nouveau_noeud;
				
		
	
	free(sequence_nodale->table);
	sequence_nodale->nb = nouvelle_sequence_nodale.nb;
	ALLOUER(sequence_nodale->table, nouvelle_sequence_nodale.nb);
	
	for(i = 0; i < sequence_nodale->nb; ++i)
	{
		sequence_nodale->table[i] = nouvelle_sequence_nodale.table[i];
	}
	
	// on recalcule les points de contrôle 
	
	Table_quadruplet n_table_nurbs;
	n_table_nurbs.nb = table_nurbs->nb + 1;
	ALLOUER (n_table_nurbs.table, n_table_nurbs.nb);
	
	for (i = 0; i < n_table_nurbs; ++i)
	{
		if (i <= table_nurbs->nb - (degre + 1) + 1){
			n_table_nurbs.table[i].x = table_nurbs->table[i].x;
			n_table_nurbs.table[i].y = table_nurbs->table[i].y;
			n_table_nurbs.table[i].z = table_nurbs->table[i].z;
		}
		else if (i >= table_nurbs->nb + 1)
		{
			n_table_nurbs.table[i].x = table_nurbs->table[i - 1].x;
			n_table_nurbs.table[i].y = table_nurbs->table[i - 1].y;
			n_table_nurbs.table[i].z = table_nurbs->table[i - 1].z;	
		}
		else
		{
			double a = nouveau_noeud - sequence_nodale[i];
			a /= sequence_nodale[table_nurbs->nb + (*degre)] - sequence_nodale[i];

			n_table_nurbs.table[i].x = (1 - a) * table_nurbs->table[i - 1].x + a * table_nurbs->table[i].x;
			n_table_nurbs.table[i].y = (1 - a) * table_nurbs->table[i - 1].y + a * table_nurbs->table[i].y;
			n_table_nurbs.table[i].z = (1 - a) * table_nurbs->table[i - 1].z + a * table_nurbs->table[i].z;	
		}
	}
	//http://web.mit.edu/hyperbook/Patrikalakis-Maekawa-Cho/node18.html
	table_nurbs->table = n_table_nurbs.table;
	table_nurbs->nb = n_table_nurbs.nb;
		
}
static void changement(struct nurbs *o)
{
	if ( ! (UN_CHAMP_CHANGE(o)||CREATION(o)) )
    return ;
    
	double pas = 1.f/(o->nb_pts -1);
	
	if (CREATION(o))
	{
		o->sequence_nodale.nb = o->degre + o->table_nurbs.nb + 1;
		ALLOUER(o->sequence_nodale.table, o->sequence_nodale.nb);
			
		for (unsigned i = 0; i <= o->degre; ++i)
		{
			o->sequence_nodale.table[i] = 0;
			o->sequence_nodale.table[i + o->table_nurbs.nb] = 1;
		}
		
		for (unsigned i = o->degre + 1; i < o->table_nurbs.nb; ++i)
		{
			o->sequence_nodale.table[i] = i - o->degre;
			o->sequence_nodale.table[i] /= o->table_nurbs.nb - (o->degre + 1) + 1;
		}
		
		if (o->nb_pts < 2)
			o->nb_pts = 10;
		
		o->affichage.nb = o->nb_pts;
		ALLOUER(o->affichage.table, o->nb_pts);
		
		double u = 0.f;
		
		for(int k = 0 ; k < o->nb_pts ; k++)
		{
			int r = cacluler_r(&o->sequence_nodale, o->table_nurbs.nb, o->degre + 1, u);
			o->affichage.table[k] = calcPoint(o->table_nurbs, o->sequence_nodale, u, r, o->degre + 1);
			u += pas;
		}
	}
	
	if (CHAMP_CHANGE(o, nb_pts)){
		if (o->nb_pts < 2)
			o->nb_pts = 10;
	}
	
	if (CHAMP_CHANGE(o, nouveau_noeud)){
		inserer_noeud(&(o->table_nurbs), &(o->degre), &(o->sequence_nodale), o->nouveau_noeud);
	}
	
	if (CHAMP_CHANGE(o, degre)){
		if (o->degre < 0)
			o->degre = 0;
		if (o->degre > o->table_nurbs.nb - 1)
			o->degre = o->table_nurbs.nb - 1;
		
		o->sequence_nodale.nb = o->degre + o->table_nurbs.nb + 1;
		free(o->sequence_nodale.table);
		ALLOUER(o->sequence_nodale.table, o->sequence_nodale.nb);
			
		for (unsigned i = 0; i <= o->degre; ++i)
		{
			o->sequence_nodale.table[i] = 0;
			o->sequence_nodale.table[i + o->table_nurbs.nb] = 1;
		}
		
		for (unsigned i = o->degre + 1; i < o->table_nurbs.nb; ++i)
		{
			o->sequence_nodale.table[i] = i - o->degre ;
			o->sequence_nodale.table[i] /= o->table_nurbs.nb - (o->degre + 1) + 1;
		}	
	}
	
	if (CHAMP_CHANGE(o, nouveau_noeud) || CHAMP_CHANGE(o, nb_pts) || CHAMP_CHANGE(o, degre) || CHAMP_CHANGE(o, table_nurbs)) {
		
		o->affichage.nb = o->nb_pts;
		free(o->affichage.table);
		ALLOUER(o->affichage.table, o->nb_pts);
		
		double u = 0.f;
		
		for(int k=0 ; k < o->nb_pts ; k++)
		{
			int r = cacluler_r(&o->sequence_nodale, o->table_nurbs.nb, o->degre + 1, u);
			o->affichage.table[k] = calcPoint(o->table_nurbs, o->sequence_nodale, u, r, o->degre + 1);
			u += pas;
		}
	}

}

CLASSE(nurbs, struct nurbs,      
       CHAMP(table_nurbs, L_table_point P_table_quadruplet Sauve Extrait)   
       CHAMP(degre, LABEL("degre") L_entier Edite Sauve DEFAUT("2"))
       CHAMP(nb_pts, LABEL("Nombre de points") L_entier  Edite Sauve DEFAUT("20") )
       CHAMP(sequence_nodale, LABEL("Sequence Nodale") L_table_flottant P_table_flottant Affiche Extrait)
       CHAMP(nouveau_noeud, LABEL("Nouveau noeud") L_flottant Edite Sauve)
       CHAMP(polygone_ctrl, LABEL("afficher le polygone de contrôle") L_booleen  Edite Sauve DEFAUT("0") )
       CHANGEMENT(changement)
       CHAMP_VIRTUEL(L_affiche_gl(affiche_nurbs))
       
       MENU("TP_PERSO/nurbs")
       EVENEMENT("Ctrl+NU")
       )
       
