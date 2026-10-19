<<<<<<< HEAD
#ifndef ITEM_H
#define ITEM_H

typedef struct item ITEM;

#endif 
=======
#ifndef ITEM_H
#define ITEM_H

typedef struct item ITEM;

ITEM* item_criar(char asteroid_designation[50]; char asteroid_fullname[100]; char close_approach_date[30]; 
                int year; int month; double distance_au; double velocity_km_s; double absolute_magnitude; 
                int days_until_approach; int is_past_event; int is_future_event; double risk_score; 
                int panic_level; char threat_category[30]; char panic_verdict[200]; int on_sentry_list; 
                double sentry_impact_prob; double sentry_torino_scale; double sentry_palermo_scale; 
                double sentry_diameter_km;);
void item_apagar(ITEM** item);
void item_escrever_csv(ITEM* item);

#endif 
>>>>>>> a0de4d175a37e5f744049109f80c6a7ee0ed20a3
