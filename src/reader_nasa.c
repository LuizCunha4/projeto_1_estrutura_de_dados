#include "reader_privado.h"

READER *reader_nasa_criar(FILE *arquivo) {
    /* O leitor conhece os tipos; os TADs recebem somente ITEMs genericos.
     * Datas/identificadores permanecem textos e campos vazios sao ausentes. */
    static const READER_COLUNA colunas[] = {
#define C(nome, tipo) {nome, nome, tipo, false}
        C("asteroid_designation", CAMPO_TEXTO),
        C("asteroid_fullname", CAMPO_TEXTO),
        C("close_approach_date", CAMPO_TEXTO),
        C("year", CAMPO_INTEIRO), C("month", CAMPO_INTEIRO),
        C("distance_au", CAMPO_REAL), C("velocity_km_s", CAMPO_REAL),
        C("absolute_magnitude", CAMPO_REAL), C("days_until_approach", CAMPO_INTEIRO),
        C("is_past_event", CAMPO_BOOLEANO), C("is_future_event", CAMPO_BOOLEANO),
        C("risk_score", CAMPO_REAL), C("panic_level", CAMPO_INTEIRO),
        C("threat_category", CAMPO_TEXTO), C("panic_verdict", CAMPO_TEXTO),
        C("on_sentry_list", CAMPO_BOOLEANO), C("sentry_impact_prob", CAMPO_REAL),
        C("sentry_torino_scale", CAMPO_REAL), C("sentry_palermo_scale", CAMPO_REAL),
        C("sentry_diameter_km", CAMPO_REAL)
#undef C
    };
    return reader_criar_interno(arquivo, ',', colunas,
                               sizeof(colunas) / sizeof(colunas[0]));
}
    