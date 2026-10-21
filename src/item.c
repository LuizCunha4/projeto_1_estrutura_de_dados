#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "item.h"

struct item {
    char asteroid_designation[50];
    char asteroid_fullname[100];
    char close_approach_date[30];
    int year;
    int month;
    double distance_au;
    double velocity_km_s;
    double absolute_magnitude;
    int days_until_approach;
    int is_past_event;
    int is_future_event;
    double risk_score;
    int panic_level;
    char threat_category[30];
    char panic_verdict[200];
    int on_sentry_list;
    double sentry_impact_prob;
    double sentry_torino_scale;
    double sentry_palermo_scale;
    double sentry_diameter_km;
};

static void copiar_string(char *destino, size_t capacidade, const char *origem) {
    if (origem == NULL) {
        destino[0] = '\0';
        return;
    }

    strncpy(destino, origem, capacidade - 1);
    destino[capacidade - 1] = '\0';
}

ITEM *item_criar(const char *asteroid_designation,
                 const char *asteroid_fullname,
                 const char *close_approach_date,
                 int year,
                 int month,
                 double distance_au,
                 double velocity_km_s,
                 double absolute_magnitude,
                 int days_until_approach,
                 int is_past_event,
                 int is_future_event,
                 double risk_score,
                 int panic_level,
                 const char *threat_category,
                 const char *panic_verdict,
                 int on_sentry_list,
                 double sentry_impact_prob,
                 double sentry_torino_scale,
                 double sentry_palermo_scale,
                 double sentry_diameter_km) {
    ITEM *item = malloc(sizeof(*item));
    if (item == NULL) {
        return NULL;
    }

    copiar_string(item->asteroid_designation, sizeof(item->asteroid_designation), asteroid_designation);
    copiar_string(item->asteroid_fullname, sizeof(item->asteroid_fullname), asteroid_fullname);
    copiar_string(item->close_approach_date, sizeof(item->close_approach_date), close_approach_date);
    copiar_string(item->threat_category, sizeof(item->threat_category), threat_category);
    copiar_string(item->panic_verdict, sizeof(item->panic_verdict), panic_verdict);

    item->year = year;
    item->month = month;
    item->distance_au = distance_au;
    item->velocity_km_s = velocity_km_s;
    item->absolute_magnitude = absolute_magnitude;
    item->days_until_approach = days_until_approach;
    item->is_past_event = is_past_event;
    item->is_future_event = is_future_event;
    item->risk_score = risk_score;
    item->panic_level = panic_level;
    item->on_sentry_list = on_sentry_list;
    item->sentry_impact_prob = sentry_impact_prob;
    item->sentry_torino_scale = sentry_torino_scale;
    item->sentry_palermo_scale = sentry_palermo_scale;
    item->sentry_diameter_km = sentry_diameter_km;

    return item;
}

void item_apagar(ITEM **item) {
    if (item == NULL) {
        return;
    }

    free(*item);
    *item = NULL;
}

void item_escrever_csv(const ITEM *item, FILE *arquivo) {
    if (item == NULL || arquivo == NULL) {
        return;
    }

    fprintf(arquivo,
            "%s,%s,%s,%d,%d,%.6f,%.6f,%.6f,%d,%d,%d,%.6f,%d,%s,%s,%d,%.6f,%.6f,%.6f,%.6f\n",
            item->asteroid_designation,
            item->asteroid_fullname,
            item->close_approach_date,
            item->year,
            item->month,
            item->distance_au,
            item->velocity_km_s,
            item->absolute_magnitude,
            item->days_until_approach,
            item->is_past_event,
            item->is_future_event,
            item->risk_score,
            item->panic_level,
            item->threat_category,
            item->panic_verdict,
            item->on_sentry_list,
            item->sentry_impact_prob,
            item->sentry_torino_scale,
            item->sentry_palermo_scale,
            item->sentry_diameter_km);
}
