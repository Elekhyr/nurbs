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

static Triplet calcPoint(Table_quadruplet pts_nurbs, double u)
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
	
	for(int rang = 1; rang < pts_nurbs.nb ; rang++) {
		//de casteljau
		
		for(int i=0 ; i < pts_nurbs.nb - rang ; i++) {
			T.table[i].x = T.table[i].x * (1-u) + T.table[i+1].x * u;
			T.table[i].y = T.table[i].y * (1-u) + T.table[i+1].y * u;
			T.table[i].z = T.table[i].z * (1-u) + T.table[i+1].z * u;
			T.table[i].h = T.table[i].h * (1-u) + T.table[i+1].h * u;
		}
	}
	//projection en cartesien
	
	T.table[0].x  = T.table[0].x / T.table[0].h;
	T.table[0].y  = T.table[0].y / T.table[0].h;
	T.table[0].z  = T.table[0].z / T.table[0].h;
	
	P.x = T.table[0].x;
	P.y = T.table[0].y;
	P.z = T.table[0].z;
	free(T.table);
	return P;
}

static void affiche_nurbs(struct nurbs *o)
{
	
	glEnd();
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

static void inserer_noeud(Table_flottant *sequence_nodale, double nouveau_noeud)
{//todo
	Table_flottant nouvelle_sequence_nodale;
	nouvelle_sequence_nodale.nb = sequence_nodale->nb + 1;
	ALLOUER(nouvelle_sequence_nodale.table, nouvelle_sequence_nodale.nb);
	
	unsigned i;
	for(i = 0; i < sequence_nodale->nb / 2; ++i)
	{
		nouvelle_sequence_nodale.table[i] = sequence_nodale->table[i];
		nouvelle_sequence_nodale.table[i + sequence_nodale->nb / 2 + 1] = sequence_nodale->table[i + sequence_nodale->nb / 2];
	}
	
	nouvelle_sequence_nodale.table[i] = nouveau_noeud;
	
	free(sequence_nodale->table);
	sequence_nodale->table = nouvelle_sequence_nodale.table;
}
static void changement(struct nurbs *o)
{ 
	double u = 0.f;
	double pas = 1.f/(o->nb_pts -1);
	
	if (CREATION(o))
	{
		o->sequence_nodale.nb = o->degre + o->table_nurbs.nb + 1;
		ALLOUER(o->sequence_nodale.table, o->sequence_nodale.nb);
		
		for (unsigned i = 0; i < o-> sequence_nodale.nb; ++i)
			o->sequence_nodale.table[i] = 0;
			
		for (unsigned i = 0; i <= o->degre; ++i)
		{
			o->sequence_nodale.table[i] = 0;
			o->sequence_nodale.table[i + o->table_nurbs.nb] = 1;
		}
		
		for (unsigned i = o->degre + 1; i < o->table_nurbs.nb; ++i)
		{
			o->sequence_nodale.table[i] = i - o->degre + 1;
			o->sequence_nodale.table[i] /= o->table_nurbs.nb - o->degre + 2;
		}	
	}
	
	if (CHAMP_CHANGE(o, nouveau_noeud))
		inserer_noeud(&(o->sequence_nodale), o->nouveau_noeud);
		
	if (CHAMP_CHANGE(o, degre)){
		o->sequence_nodale.nb = o->degre + o->table_nurbs.nb + 1;
		free(o->sequence_nodale.table);
		ALLOUER(o->sequence_nodale.table, o->sequence_nodale.nb);
		
		for (unsigned i = 0; i < o-> sequence_nodale.nb; ++i)
			o->sequence_nodale.table[i] = 0;
			
		for (unsigned i = 0; i <= o->degre; ++i)
		{
			o->sequence_nodale.table[i] = 0;
			o->sequence_nodale.table[i + o->table_nurbs.nb] = 1;
		}
		
		for (unsigned i = o->degre + 1; i < o->table_nurbs.nb; ++i)
		{
			o->sequence_nodale.table[i] = i - o->degre + 1;
			o->sequence_nodale.table[i] /= o->table_nurbs.nb - o->degre + 2;
		}
	}

	ALLOUER(o->affichage.table, o->nb_pts);
	
	if (o->nb_pts < 2)
		o->nb_pts = 10;
	
	for(int k=0 ; k < o->nb_pts ; k++)
	{
		o->affichage.table[k] = calcPoint(o->table_nurbs, u);
		u += pas;
	}
	
	
}

CLASSE(nurbs, struct nurbs,      
       CHAMP(table_nurbs, L_table_point P_table_quadruplet Sauve Extrait)   
       CHAMP(degre, LABEL("degre") L_entier Edite Sauve DEFAUT("2"))
       CHAMP(nb_pts, LABEL("Nombre de points") L_entier  Edite Sauve DEFAUT("10") )
       CHAMP(sequence_nodale, LABEL("Sequence Nodale") L_table_flottant P_table_flottant Affiche Extrait)
       CHAMP(nouveau_noeud, LABEL("Nouveau noeud") L_flottant Edite Sauve)
       CHAMP(polygone_ctrl, LABEL("afficher le polygone de contrôle") L_booleen  Edite Sauve DEFAUT("0") )
       CHANGEMENT(changement)
       CHAMP_VIRTUEL(L_affiche_gl(affiche_nurbs))
       
       MENU("TP_PERSO/nurbs")
       EVENEMENT("Ctrl+BR")
       )
       
