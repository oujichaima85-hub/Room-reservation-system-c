#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SALLES 50
#define MAX_RESERVATIONS 1000

typedef enum {
    STATUT_CONFIRMEE = 1,
    STATUT_ANNULEE = 2,
    STATUT_FACTUREE = 3
} StatutReservation;

typedef struct {
    char nom[50];
    int capacite;
    double tarif_horaire;
    char equipements[200];
} Salle;

typedef struct {
    int id;
    char nom_client[100];
    char nom_salle[50];
    char date[11];      /* YYYY-MM-DD */
    char heure_debut[6]; /* HH:MM */
    char heure_fin[6];
    int nombre_personnes;
    double tarif_horaire;
    double montant_total;
    StatutReservation statut;
} Reservation;

Salle salles[MAX_SALLES];
int nbSalles = 0;

Reservation reservations[MAX_RESERVATIONS];
int nbReservations = 0;
int prochainIdReservation = 1;

/* Prototypes */
void chargerSalles(const char *nomFichier);
void chargerReservations(const char *nomFichier);
void sauvegarderReservations(const char *nomFichier);
void afficherSalle(const Salle *s);
void afficherToutesSalles();
int trouverSalleParNom(const char *nom);
int choisirSalle();
int parseHeureMinutes(const char *heure);
double calculerDureeHeures(const char *heureDebut, const char *heureFin);
int verifieConflit(const char *nomSalle, const char *date, const char *heureDebut, const char *heureFin);
const char* statutToString(StatutReservation s);
void afficherReservation(const Reservation *r);
void afficherToutesReservations();
void creerReservation();
void annulerReservation();
void genererFacturePourReservation(Reservation *r);
void marquerReservationFacturee();
void menuReservations();
void statsChiffreAffairesParSalle();
void statsReservationsParMois();
void statsSallesPopulaires();
void menuStatistiques();

int main(void) {
    chargerSalles("config_salles.txt");
    chargerReservations("reservations.txt");

    int choix = -1;
    while (choix != 0) {
        printf("\n=== Gestion des reservations de salles ===\n");
        printf("1. Afficher les salles\n");
        printf("2. Gestion des reservations\n");
        printf("3. Statistiques\n");
        printf("0. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                afficherToutesSalles();
                break;
            case 2:
                menuReservations();
                break;
            case 3:
                menuStatistiques();
                break;
            case 0:
                break;
            default:
                printf("Choix invalide.\n");
        }
    }

    sauvegarderReservations("reservations.txt");
    printf("Sauvegarde des donnees et fermeture...\n");
    return 0;
}

void chargerSalles(const char *nomFichier) {
    FILE *f = fopen(nomFichier, "r");
    if (!f) {
        printf("Impossible d'ouvrir le fichier de salles %s.\n", nomFichier);
        nbSalles = 0;
        return;
    }

    nbSalles = 0;
    char ligne[512];
    while (fgets(ligne, sizeof(ligne), f) && nbSalles < MAX_SALLES) {
        Salle s;
        char *p = strchr(ligne, '\n');
        if (p) *p = '\0';

        char *token = strtok(ligne, ";");
        if (!token) continue;
        strncpy(s.nom, token, sizeof(s.nom));
        s.nom[sizeof(s.nom) - 1] = '\0';

        token = strtok(NULL, ";");
        if (!token) continue;
        s.capacite = atoi(token);

        token = strtok(NULL, ";");
        if (!token) continue;
        s.tarif_horaire = atof(token);

        token = strtok(NULL, ";");
        if (!token) token = "";
        strncpy(s.equipements, token, sizeof(s.equipements));
        s.equipements[sizeof(s.equipements) - 1] = '\0';

        salles[nbSalles++] = s;
    }

    fclose(f);
}

void chargerReservations(const char *nomFichier) {
    FILE *f = fopen(nomFichier, "r");
    if (!f) {
        nbReservations = 0;
        prochainIdReservation = 1;
        return;
    }

    nbReservations = 0;
    char ligne[512];
    int maxId = 0;

    while (fgets(ligne, sizeof(ligne), f) && nbReservations < MAX_RESERVATIONS) {
        Reservation r;
        int statutInt;
        int nbLus = sscanf(ligne,
                           "%d;%99[^;];%49[^;];%10[^;];%5[^;];%5[^;];%d;%lf;%lf;%d",
                           &r.id,
                           r.nom_client,
                           r.nom_salle,
                           r.date,
                           r.heure_debut,
                           r.heure_fin,
                           &r.nombre_personnes,
                           &r.tarif_horaire,
                           &r.montant_total,
                           &statutInt);
        if (nbLus == 10) {
            r.statut = (StatutReservation)statutInt;
            reservations[nbReservations++] = r;
            if (r.id > maxId) {
                maxId = r.id;
            }
        }
    }

    fclose(f);
    prochainIdReservation = maxId + 1;
    if (prochainIdReservation < 1) {
        prochainIdReservation = 1;
    }
}

void sauvegarderReservations(const char *nomFichier) {
    FILE *f = fopen(nomFichier, "w");
    if (!f) {
        printf("Erreur : impossible de sauvegarder dans %s\n", nomFichier);
        return;
    }

    for (int i = 0; i < nbReservations; ++i) {
        Reservation *r = &reservations[i];
        fprintf(f, "%d;%s;%s;%s;%s;%s;%d;%.2f;%.2f;%d\n",
                r->id,
                r->nom_client,
                r->nom_salle,
                r->date,
                r->heure_debut,
                r->heure_fin,
                r->nombre_personnes,
                r->tarif_horaire,
                r->montant_total,
                (int)r->statut);
    }

    fclose(f);
}

void afficherSalle(const Salle *s) {
    printf("Salle: %s | Capacite: %d | Tarif horaire: %.2f | Equipements: %s\n",
           s->nom, s->capacite, s->tarif_horaire, s->equipements);
}

void afficherToutesSalles() {
    printf("\n=== Liste des salles ===\n");
    if (nbSalles == 0) {
        printf("Aucune salle trouvee.\n");
        return;
    }
    for (int i = 0; i < nbSalles; ++i) {
        printf("%d. ", i + 1);
        afficherSalle(&salles[i]);
    }
}

int trouverSalleParNom(const char *nom) {
    for (int i = 0; i < nbSalles; ++i) {
        if (strcmp(salles[i].nom, nom) == 0) {
            return i;
        }
    }
    return -1;
}

int choisirSalle() {
    if (nbSalles == 0) {
        printf("Aucune salle disponible.\n");
        return -1;
    }

    int choix;
    afficherToutesSalles();
    printf("Choisissez une salle (numero) : ");
    scanf("%d", &choix);
    if (choix < 1 || choix > nbSalles) {
        printf("Choix invalide.\n");
        return -1;
    }
    return choix - 1;
}

int parseHeureMinutes(const char *heure) {
    int h, m;
    if (sscanf(heure, "%d:%d", &h, &m) != 2) {
        return -1;
    }
    if (h < 0 || h > 23 || m < 0 || m > 59) {
        return -1;
    }
    return h * 60 + m;
}

double calculerDureeHeures(const char *heureDebut, const char *heureFin) {
    int debut = parseHeureMinutes(heureDebut);
    int fin = parseHeureMinutes(heureFin);
    if (debut < 0 || fin < 0 || fin <= debut) {
        return -1.0;
    }
    return (double)(fin - debut) / 60.0;
}

int verifieConflit(const char *nomSalle, const char *date, const char *heureDebut, const char *heureFin) {
    int debutN = parseHeureMinutes(heureDebut);
    int finN = parseHeureMinutes(heureFin);
    if (debutN < 0 || finN < 0) {
        return 1; /* considere comme conflit si heure invalide */
    }

    for (int i = 0; i < nbReservations; ++i) {
        Reservation *r = &reservations[i];
        if (strcmp(r->nom_salle, nomSalle) == 0 &&
            strcmp(r->date, date) == 0 &&
            r->statut != STATUT_ANNULEE) {
            int debutE = parseHeureMinutes(r->heure_debut);
            int finE = parseHeureMinutes(r->heure_fin);
            if (debutE < 0 || finE < 0) {
                continue;
            }
            if (debutN < finE && finN > debutE) {
                return 1; /* chevauchement */
            }
        }
    }
    return 0;
}

const char* statutToString(StatutReservation s) {
    switch (s) {
        case STATUT_CONFIRMEE: return "CONFIRMEE";
        case STATUT_ANNULEE: return "ANNULEE";
        case STATUT_FACTUREE: return "FACTUREE";
        default: return "INCONNU";
    }
}

void afficherReservation(const Reservation *r) {
    printf("ID: %d | Client: %s | Salle: %s | Date: %s %s-%s | Pers: %d | Tarif h: %.2f | Total: %.2f | Statut: %s\n",
           r->id,
           r->nom_client,
           r->nom_salle,
           r->date,
           r->heure_debut,
           r->heure_fin,
           r->nombre_personnes,
           r->tarif_horaire,
           r->montant_total,
           statutToString(r->statut));
}

void afficherToutesReservations() {
    printf("\n=== Liste des reservations ===\n");
    if (nbReservations == 0) {
        printf("Aucune reservation.\n");
        return;
    }
    for (int i = 0; i < nbReservations; ++i) {
        afficherReservation(&reservations[i]);
    }
}

void creerReservation() {
    if (nbReservations >= MAX_RESERVATIONS) {
        printf("Nombre maximal de reservations atteint.\n");
        return;
    }

    int idxSalle = choisirSalle();
    if (idxSalle < 0) {
        return;
    }

    Salle *salle = &salles[idxSalle];
    Reservation r;
    r.id = prochainIdReservation++;

    printf("Nom du client : ");
    scanf(" %99[^\n]", r.nom_client);

    strncpy(r.nom_salle, salle->nom, sizeof(r.nom_salle));
    r.nom_salle[sizeof(r.nom_salle) - 1] = '\0';

    printf("Date (YYYY-MM-DD) : ");
    scanf("%10s", r.date);

    printf("Heure debut (HH:MM) : ");
    scanf("%5s", r.heure_debut);

    printf("Heure fin (HH:MM) : ");
    scanf("%5s", r.heure_fin);

    printf("Nombre de personnes : ");
    scanf("%d", &r.nombre_personnes);

    if (r.nombre_personnes > salle->capacite) {
        printf("Erreur : la salle ne peut pas accueillir autant de personnes (capacite %d).\n", salle->capacite);
        return;
    }

    if (verifieConflit(r.nom_salle, r.date, r.heure_debut, r.heure_fin)) {
        printf("Erreur : la salle est deja reservee sur ce creneau.\n");
        return;
    }

    double duree = calculerDureeHeures(r.heure_debut, r.heure_fin);
    if (duree <= 0) {
        printf("Erreur : duree invalide.\n");
        return;
    }

    r.tarif_horaire = salle->tarif_horaire;
    r.montant_total = r.tarif_horaire * duree;
    r.statut = STATUT_CONFIRMEE;

    reservations[nbReservations++] = r;
    sauvegarderReservations("reservations.txt");

    printf("Reservation creee avec succes. ID = %d, montant = %.2f EUR, duree = %.2f h.\n",
           r.id, r.montant_total, duree);
}

void annulerReservation() {
    if (nbReservations == 0) {
        printf("Aucune reservation a annuler.\n");
        return;
    }

    int id;
    printf("ID de la reservation a annuler : ");
    scanf("%d", &id);

    for (int i = 0; i < nbReservations; ++i) {
        if (reservations[i].id == id) {
            if (reservations[i].statut == STATUT_ANNULEE) {
                printf("Reservation deja annulee.\n");
                return;
            }
            reservations[i].statut = STATUT_ANNULEE;
            sauvegarderReservations("reservations.txt");
            printf("Reservation %d annulee.\n", id);
            return;
        }
    }

    printf("Reservation avec ID %d introuvable.\n", id);
}

void genererFacturePourReservation(Reservation *r) {
    char nomFichier[64];
    snprintf(nomFichier, sizeof(nomFichier), "FACTURE_%d.txt", r->id);

    FILE *f = fopen(nomFichier, "w");
    if (!f) {
        printf("Erreur : impossible de creer la facture %s\n", nomFichier);
        return;
    }

    double duree = calculerDureeHeures(r->heure_debut, r->heure_fin);

    fprintf(f, "Facture pour reservation %d\n", r->id);
    fprintf(f, "Client : %s\n", r->nom_client);
    fprintf(f, "Salle : %s\n", r->nom_salle);
    fprintf(f, "Date : %s\n", r->date);
    fprintf(f, "Horaire : %s - %s\n", r->heure_debut, r->heure_fin);
    fprintf(f, "Duree (heures) : %.2f\n", duree);
    fprintf(f, "Tarif horaire : %.2f EUR\n", r->tarif_horaire);
    fprintf(f, "Montant total : %.2f EUR\n", r->montant_total);

    fclose(f);
    printf("Facture generee dans le fichier %s\n", nomFichier);
}

void marquerReservationFacturee() {
    if (nbReservations == 0) {
        printf("Aucune reservation.\n");
        return;
    }

    int id;
    printf("ID de la reservation a facturer : ");
    scanf("%d", &id);

    for (int i = 0; i < nbReservations; ++i) {
        if (reservations[i].id == id) {
            if (reservations[i].statut == STATUT_ANNULEE) {
                printf("Impossible de facturer une reservation annulee.\n");
                return;
            }
            genererFacturePourReservation(&reservations[i]);
            reservations[i].statut = STATUT_FACTUREE;
            sauvegarderReservations("reservations.txt");
            printf("Reservation %d marquee comme FACTUREE.\n", id);
            return;
        }
    }

    printf("Reservation avec ID %d introuvable.\n", id);
}

void menuReservations() {
    int choix = -1;
    while (choix != 0) {
        printf("\n=== Menu Reservations ===\n");
        printf("1. Creer une reservation\n");
        printf("2. Afficher toutes les reservations\n");
        printf("3. Annuler une reservation\n");
        printf("4. Facturer une reservation\n");
        printf("0. Retour\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                creerReservation();
                break;
            case 2:
                afficherToutesReservations();
                break;
            case 3:
                annulerReservation();
                break;
            case 4:
                marquerReservationFacturee();
                break;
            case 0:
                break;
            default:
                printf("Choix invalide.\n");
        }
    }
}

void statsChiffreAffairesParSalle() {
    printf("\n=== Chiffre d'affaires par salle ===\n");
    for (int i = 0; i < nbSalles; ++i) {
        double ca = 0.0;
        for (int j = 0; j < nbReservations; ++j) {
            Reservation *r = &reservations[j];
            if (strcmp(r->nom_salle, salles[i].nom) == 0 &&
                r->statut != STATUT_ANNULEE) {
                ca += r->montant_total;
            }
        }
        printf("%s : %.2f EUR\n", salles[i].nom, ca);
    }
}

void statsReservationsParMois() {
    printf("\n=== Nombre de reservations par mois (toute annee confondue) ===\n");
    int nbParMois[13];
    for (int i = 0; i < 13; ++i) {
        nbParMois[i] = 0;
    }

    for (int i = 0; i < nbReservations; ++i) {
        Reservation *r = &reservations[i];
        if (r->statut == STATUT_ANNULEE) {
            continue;
        }
        int annee, mois, jour;
        if (sscanf(r->date, "%d-%d-%d", &annee, &mois, &jour) == 3) {
            if (mois >= 1 && mois <= 12) {
                nbParMois[mois]++;
            }
        }
    }

    for (int m = 1; m <= 12; ++m) {
        printf("Mois %d : %d reservations\n", m, nbParMois[m]);
    }
}

void statsSallesPopulaires() {
    printf("\n=== Salles les plus populaires (par nombre de reservations) ===\n");
    if (nbSalles == 0) {
        printf("Aucune salle.\n");
        return;
    }

    for (int i = 0; i < nbSalles; ++i) {
        int nb = 0;
        for (int j = 0; j < nbReservations; ++j) {
            Reservation *r = &reservations[j];
            if (r->statut == STATUT_ANNULEE) {
                continue;
            }
            if (strcmp(r->nom_salle, salles[i].nom) == 0) {
                nb++;
            }
        }
        printf("%s : %d reservations\n", salles[i].nom, nb);
    }
}

void menuStatistiques() {
    int choix = -1;
    while (choix != 0) {
        printf("\n=== Menu Statistiques ===\n");
        printf("1. Chiffre d'affaires par salle\n");
        printf("2. Nombre de reservations par mois\n");
        printf("3. Salles les plus populaires\n");
        printf("0. Retour\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                statsChiffreAffairesParSalle();
                break;
            case 2:
                statsReservationsParMois();
                break;
            case 3:
                statsSallesPopulaires();
                break;
            case 0:
                break;
            default:
                printf("Choix invalide.\n");
        }
    }
}
