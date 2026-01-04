
#include "psicrometria.h"
#include <math.h>
volatile double PATM = 101.325;
void WINAPI set_patm_at_altitude(double altitude) {
     // Patm = 101325 * (1 - 2.25577 * 10^-5 * Quota) ^ 5.2559
     PATM = 101.325 * pow(1.0 - 2.25577e-5 * altitude, 5.2559);
 }


// --- FORMULE PSICROMETRICHE ---
PSICRO_API Psat(double t) {
    double T = t + 273.15;
    double lnPs;
    if (t >= 0.0) {
        // ASHRAE Fundamentals - Hyland & Wexler
        double C8  = -5800.2206;
        double C9  = 1.3914993;
        double C10 = -0.048640239;
        double C11 = 0.000041764768;
        double C12 = -0.000000014452093;
        double C13 = 6.5459673;
        lnPs = (C8 / T) + C9 + (C10 * T) + (C11 * T * T) + (C12 * T * T * T) + (C13 * log(T));
    } else {
        // ASHRAE per T < 0 (Ghiaccio)
        double C1 = -5674.5359;
        double C2 = 6.3925247;
        double C3 = -0.009677843;
        double C4 = 0.00000062215701;
        double C5 = 0.0000000020747825;
        double C6 = -0.0000000000009484024;
        double C7 = 4.1635019;
        lnPs = (C1 / T) + C2 + (C3 * T) + (C4 * T * T) + (C5 * T * T * T) + (C6 * T * T * T * T) + (C7 * log(T));
    }
    return exp(lnPs) / 1000.0; // Restituisce kPa
}
// --- FUNZIONE INVERSA TEMPERATURA DI SATURAZIONE ---
PSICRO_API TPsat(double p_kpa) {
    if (p_kpa <= 0.0001) return -50.0; // Limite inferiore ragionevole

    // 1. IPOTESI INIZIALE (Inversione formula di Magnus)
    double lnP = log(p_kpa / 0.61078);
    double t_curr = (237.3 * lnP) / (17.27 - lnP);

    // 2. PARAMETRI DI CONVERGENZA
    double epsilon = 0.00001;
    int max_iter = 100;
    int iter = 0;
    double diff = 1.0; // Valore iniziale per far partire il ciclo
    double delta_t = 0.001; // Passo per la derivata numerica

    // 3. CICLO DI ITERAZIONE
    while (diff > epsilon && iter < max_iter) {
        double p_calc = Psat(t_curr);
        
        // Calcolo derivata numerica dP/dt
        double p_plus = Psat(t_curr + delta_t);
        double dPdt = (p_plus - p_calc) / delta_t;
        
        if (fabs(dPdt) < 1e-12) break; // Protezione divisione per zero

        // Calcolo del nuovo valore
        double t_next = t_curr - (p_calc - p_kpa) / dPdt;
        
        // Calcolo della differenza in modulo tra due soluzioni successive
        diff = fabs(t_next - t_curr);
        
        // Aggiornamento per la prossima iterazione
        t_curr = t_next;
        iter++;
    }

    return t_curr;
}
// --- TITOLO DI SATURAZIONE ALLA TEMPERATURA t ---
PSICRO_API xsat_t(double t) {
    double ps = Psat(t);
    
    // Se la pressione di saturazione si avvicina troppo alla pressione atmosferica
    // (punto di ebollizione), il titolo tende all'infinito.
    // Mettiamo un controllo di sicurezza.
    if (ps >= PATM) return 9.999; // Valore di saturazione estremo (quasi 10 kg/kg)

    // Formula: x = 0.621945 * Psat / (Patm - Psat)
    return (RAV * ps) / (PATM - ps);
}
// --- TARGET 0: TEMPERATURA (t) ---
PSICRO_API t_ur_x(double ur, double x) {
    if (ur <= 0) return -999.0; // Evita divisione per zero
    double ps = (x * PATM) / ((ur / 100.0) * (RAV + x));
    return TPsat(ps);}
PSICRO_API t_ur_h(double ur, double h) {
    // 1. STIMA INIZIALE PIÙ PRECISA
    // L'entalpia è h = 1.006*t + x*(2501 + 1.86*t). 
    // Come prima stima usiamo t = h / 1.006 (aria secca)
    double t_curr = h / CPAS;
    
    double epsilon = 0.00001;
    int max_iter = 100;
    int iter = 0;
    double diff = 1.0;
    double dt = 0.001;

    while (diff > epsilon && iter < max_iter) {
        // --- CALCOLO f(t) ---
        double ps = Psat(t_curr);
        double pv = (ur / 100.0) * ps;
        // Protezione fisica: pv non può superare PATM
        if(pv >= PATM * 0.9) pv = PATM * 0.9; 
        
        double x = RAV * pv / (PATM - pv);
        // h = cpa*t + x*(lambda + cpv*t)
        double f_t = (CPAS * t_curr + x * (LAMBDA + CPV * t_curr)) - h;

        // --- CALCOLO f(t + dt) per la derivata ---
        double ps_dt = Psat(t_curr + dt);
        double pv_dt = (ur / 100.0) * ps_dt;
        if(pv_dt >= PATM * 0.9) pv_dt = PATM * 0.9;
        
        double x_dt = RAV * pv_dt / (PATM - pv_dt);
        double f_t_dt = (CPAS * (t_curr + dt) + x_dt * (LAMBDA + CPV * (t_curr + dt))) - h;

        double dfdt = (f_t_dt - f_t) / dt;

        if (fabs(dfdt) < 1e-10) break;

        double t_next = t_curr - f_t / dfdt;
        
        // Limite di sicurezza per evitare che Newton "scappi" a temperature assurde
        if (t_next > 100) t_next = 100;
        if (t_next < -50) t_next = -50;

        diff = fabs(t_next - t_curr);
        t_curr = t_next;
        iter++;
    }

    return t_curr;
}
PSICRO_API t_ur_vau(double ur, double vau){
    double t_min = -50.0;
    double t_max = 100.0;
    double t_mid = 0.0;
    double t_old = 1e9;      /* valore fittizio iniziale */

    double eps_vau = 1e-5;   /* tolleranza su volume specifico */
    double eps_t   = 1e-5;   /* tolleranza su temperatura */

    int max_iter = 100;
    int iter = 0;

    while (iter < max_iter)
    {
        t_mid = 0.5 * (t_min + t_max);

        /* criterio su differenza temperatura */
        if (fabs(t_mid - t_old) < eps_t)
            break;

        double vau_mid = vau_t_ur(t_mid, ur);

        /* criterio su volume specifico */
        if (fabs(vau_mid - vau) < eps_vau)
            break;

        if (vau_mid > vau)
            t_max = t_mid;
        else
            t_min = t_mid;

        t_old = t_mid;
        iter++;
    }

    return t_mid;}
PSICRO_API t_ur_tbu(double ur, double tbu){ return t_ur_h(ur,h_ur_tbu(ur,tbu));}
PSICRO_API t_ur_tr(double ur, double tr){  return t_ur_x(ur, x_t_ur(tr,100)); }
PSICRO_API t_x_h(double x, double h){return ((h-x*LAMBDA)/(CPAS+x*CPV));}
PSICRO_API t_x_vau(double x, double vau){return vau * (PATM/ RA) / (1 + (RV/RA) * x) - 273.15;}
PSICRO_API t_x_tbu(double x, double tbu){ return (t_x_h(x, h_x_tbu(x, tbu))); }                                            
PSICRO_API t_x_tr(double x, double tr) { 
    (void)x; // Evita warning compilatore
    return tr; 
}
PSICRO_API t_h_vau(double h, double vau){
    /* SOLUZIONE ANALITICA EQ. COMBINATA (28)+(32)
       Polinomio di 2° grado in T */
    double c1, c2, c3;
    double delta;
    double t1 = 0.0;
    double t2 = 0.0;

    c1 = CPV - CPAS / RAV;
    c2 = 273.15 * (CPV - CPAS / RAV)
       + LAMBDA
       + h / RAV
       - vau * CPV * PATM / RA;

    c3 = 273.15 * (LAMBDA + h / RAV)
       - LAMBDA * vau * PATM / RA;

    delta = c2 * c2 - 4.0 * c1 * c3;

    if (delta < 0.0)
    {
        /* Errore fisico/numerico */
        return NAN;   /* oppure -999.0 se preferisci */
    }

    t1 = (-c2 + sqrt(delta)) / (2.0 * c1);
    t2 = (-c2 - sqrt(delta)) / (2.0 * c1);

    /* restituisce la soluzione fisicamente significativa */
    return (t1 > t2) ? t1 : t2;}
PSICRO_API t_h_tbu(double h, double tbu){
    /* RISOLVE EQ. COMBINATA (32) + (35) A.F.H. 1997 */
    double a, b;
    b = LAMBDA - CPW * tbu;
    a = (LAMBDA - (CPW - CPV) * tbu) * xsat_t(tbu) + CPAS * tbu;
    return (a * LAMBDA - h * b)
         / (CPV * (h - a) + CPAS * (LAMBDA - b));}
PSICRO_API t_h_tr(double h, double tr){ 
    return t_x_h(x_t_ur(tr,100 ), h);}
PSICRO_API t_vau_tbu(double vau, double tbu){
    /* SOLUZIONE ANALITICA EQ. (35) + (28) A.F.H. 1997
       Polinomio di 2° grado in T */
    double c1, c2, c3;
    double delta;
    double t1, t2;
    c1 = (CPV * RA * RAV - CPAS * RA);
    c2 = (
            ((-CPW + CPV) * RA * tbu + LAMBDA * RA) * xsat_t(tbu)
          - CPV * PATM * RAV * vau
          + (-CPW * RA * RAV + CPAS * RA) * tbu
          + (LAMBDA + 273.15 * CPV) * RA * RAV
          - 273.15 * CPAS * RA
         );
    c3 = (
            ((-273.15 * CPW + 273.15 * CPV) * RA * tbu + 273.15 * LAMBDA * RA) * xsat_t(tbu)
          + (CPW * PATM * RAV * tbu - LAMBDA * PATM * RAV) * vau
          + (-273.15 * CPW * RA * RAV + 273.15 * CPAS * RA) * tbu
          + 273.15 * LAMBDA * RA * RAV
         );
    delta = c2 * c2 - 4.0 * c1 * c3;
    if (delta < 0.0)
    {
        /* errore fisico/numerico */
        return NAN;   /* oppure -999.0 per coerenza con il resto */
    }
    t1 = (-c2 + sqrt(delta)) / (2.0 * c1);
    t2 = (-c2 - sqrt(delta)) / (2.0 * c1);

    /* soluzione fisicamente valida */
    return (t1 > t2) ? t1 : t2;
}
PSICRO_API t_vau_tr(double vau, double tr){return t_x_vau(x_t_ur(tr,100), vau); }   
PSICRO_API t_tbu_tr(double tbu, double tr){ return t_x_tbu(x_t_ur(100,tr), tbu); }
// --- TARGET 1: UMIDITÀ RELATIVA (ur) ---
PSICRO_API ur_t_x(double t, double x){ 
    double Ps = Psat(t);
    double Pv = (x * PATM) / (RAV + x);
    return (Pv / Ps) * 100.0; }
PSICRO_API ur_t_h(double t, double h){ 
    double x = (h - CPAS * t) / (LAMBDA + CPV * t);
    return ur_t_x(t, x); }
PSICRO_API ur_t_vau(double t, double vau){ 
    double T_kelvin = t + 273.15;
    double x = ((vau * PATM) / (RA * T_kelvin) - 1.0) * RAV;
    return ur_t_x(t, x); }
PSICRO_API ur_t_tbu(double t, double tbu){return ur_t_h(t, h_t_tbu(t, tbu)); }
PSICRO_API ur_t_tr(double t, double tr){return ur_t_x(t, x_t_ur(tr, 100)); }
PSICRO_API ur_x_h(double x, double h){return ur_t_h(t_x_h(x, h), h);}    
PSICRO_API ur_x_vau(double x, double vau){
    double t = (vau * PATM / (RA + RV * x)) - 273.15;
    return ur_t_x(t,x); }
PSICRO_API ur_x_tbu(double x, double tbu){return ur_x_h(x, h_x_tbu(x, tbu)); }
PSICRO_API ur_x_tr(double x, double tr){ (void)x; (void)tr; return 0; }
PSICRO_API ur_h_vau(double h, double vau){return ur_t_h(t_h_vau(h, vau), h); }
PSICRO_API ur_h_tbu(double h, double tbu){
 //   ' risolve le (32) e (35) risp a t,x e applica urxt
   double t=t_h_tbu(h, tbu);
   return ur_t_x(t, x_t_h (h, t)); }
PSICRO_API ur_h_tr(double h, double tr){return ur_x_h(x_t_ur(tr,100), h); }
PSICRO_API ur_vau_tbu(double vau, double tbu){
    return ur_t_vau(t_vau_tbu(vau, tbu),vau); }
PSICRO_API ur_vau_tr(double vau, double tr){return ur_x_vau(x_t_ur(tr,100), vau); }
PSICRO_API ur_tbu_tr(double tbu, double tr){return ur_x_tbu(x_t_ur(tr,100), tbu); }

// --- TARGET 2: TITOLO (x) ---
PSICRO_API x_t_ur(double t, double ur) {
    double Ps = Psat(t);
    double Pv = (ur / 100.0) * Ps;
    return ((RAV * Pv) / (PATM - Pv));}
PSICRO_API x_t_h(double t, double h){ return ((h - (CPAS * t)) / (LAMBDA + CPV * t)); }
PSICRO_API x_t_vau(double t, double vau){ return ((vau * PATM) / (RA * (t + 273.15)) - 1.0) * RAV; }
PSICRO_API x_t_tbu(double t, double tbu){ 
    double Ps_tbu = Psat(tbu);
    double x_sat_tbu = (RAV * Ps_tbu) / (PATM - Ps_tbu);
    double x = ((LAMBDA - (CPW - CPV) * tbu) * x_sat_tbu - CPAS * (t - tbu)) / (LAMBDA + CPV * t - CPW * tbu);
    return x; }
PSICRO_API x_t_tr(double t, double tr){ 
    (void)t;
    double Ps_tr = Psat(tr);
    return (RAV * Ps_tr) / (PATM - Ps_tr); }
PSICRO_API x_ur_h(double ur, double h){ return x_t_h(t_ur_h(ur, h), h); }
PSICRO_API x_ur_vau(double ur, double vau){return x_t_ur(t_ur_vau(ur, vau), ur); }
PSICRO_API x_ur_tbu(double ur, double tbu){return x_ur_h(ur, h_ur_tbu(ur, tbu)); }
PSICRO_API x_ur_tr(double ur, double tr){return x_t_ur(tr, 100);}
PSICRO_API x_h_vau(double h, double vau){return x_t_h(t_h_vau(h, vau), h); }
PSICRO_API x_h_tbu(double h, double tbu){ return x_t_h(t_h_tbu(h, tbu), h); }
PSICRO_API x_h_tr(double h, double tr){
  if (h_t_ur(tr,100)<h){
        return x_t_h(tr,h);}
  else{
        return x_t_ur(tr, 100);}
}
PSICRO_API x_vau_tbu(double vau, double tbu){return x_t_vau( t_vau_tbu(vau, tbu), vau); }
PSICRO_API x_vau_tr(double vau, double tr){ return x_t_ur(tr, 100); }
PSICRO_API x_tbu_tr(double tbu, double tr){
        return x_t_ur(tr, 100);}   
// --- TARGET 3: ENTALPIA (h) --- 
PSICRO_API h_t_ur(double t, double ur){ 
    double x = x_t_ur(t, ur);
    return ((CPAS * t) + (x * (LAMBDA + CPV * t)));}
PSICRO_API h_t_x(double t, double x){ return ((CPAS * t + x * (LAMBDA + CPV * t))); }
PSICRO_API h_t_vau(double t, double vau){ 
    double T_kelvin = t + 273.15;
    double x = ((vau * PATM) / (RA * T_kelvin) - 1.0) * RAV;
    return h_t_x(t, x); }
PSICRO_API h_t_tbu(double t, double tbu){ 
    double x = x_t_tbu(t, tbu);
    return h_t_x(t, x); }
PSICRO_API h_t_tr(double t, double tr){ 
    double x = x_t_tr(t, tr);
    return h_t_x(t, x); }
PSICRO_API h_ur_x(double ur, double x){return h_t_x(t_ur_x(ur, x), x); }
PSICRO_API h_ur_vau(double ur, double vau){ 
  double t = t_ur_vau(ur,vau);
  return h_t_ur(t,ur); }
PSICRO_API h_ur_tbu(double ur, double tbu)
{
    /* --- 1. STIMA INIZIALE ---
       entalpia dell’aria satura alla temperatura di bulbo umido */
    double x_sat = xsat_t(tbu);
    double h_curr = CPAS * tbu + x_sat * (LAMBDA + CPV * tbu);

    /* --- 2. PARAMETRI NUMERICI --- */
    double eps = 1e-5;
    int max_iter = 100;
    double diff = 1.0;
    double dh = 0.01;
    int iter = 0;

    while (diff > eps && iter < max_iter)
    {
        /* f(h) */
        double tbu_calc = tbu_ur_h(ur, h_curr);
        double f = tbu_calc - tbu;

        /* derivata numerica df/dh */
        double tbu_calc_dh = tbu_ur_h(ur, h_curr + dh);
        double df_dh = (tbu_calc_dh - tbu_calc) / dh;

        if (fabs(df_dh) < 1e-10)
            break;

        /* Newton-Raphson */
        double h_next = h_curr - f / df_dh;

        /* limiti fisici di sicurezza */
        if (h_next < 0.0) h_next = 0.0;
        if (h_next > 200.0) h_next = 200.0;

        diff = fabs(h_next - h_curr);
        h_curr = h_next;
        iter++;
    }

    return h_curr;
}
PSICRO_API h_ur_tr(double ur, double tr){return h_ur_x(ur,x_t_ur(tr,100));}  
PSICRO_API h_x_vau(double x, double vau){ 
    double t = t_x_vau(x,vau);
    return h_t_x(t, x); }
PSICRO_API h_x_tbu(double x, double tbu){ 
    double a,b,t;
    a = (LAMBDA - (CPW - CPV) * tbu) * xsat_t(tbu) + CPAS * tbu ;
    b = LAMBDA - CPW * tbu ;
    t = (a - x * b) / (x * CPV + CPAS) ;
    return h_t_x(t, x); }
PSICRO_API h_x_tr(double x, double tr){(void)x; (void)tr; return 0; }
PSICRO_API h_vau_tbu(double vau, double tbu){ 
    double t = t_vau_tbu(vau, tbu);
    return h_t_tbu(t, tbu);}
PSICRO_API h_vau_tr(double vau, double tr){return h_x_vau(x_t_ur(tr, 100), vau); }
PSICRO_API h_tbu_tr(double tbu, double tr){return h_t_x( t_tbu_tr(tbu, tr), x_tbu_tr(tbu, tr));} 
// --- TARGET 4: VOLUME SPECIFICO (vau) ---
PSICRO_API vau_t_ur(double t, double ur){ 
    double x = x_t_ur(t, ur);
    return (RA * (t + 273.15) * (1.0 + (x / RAV)) / PATM); }
PSICRO_API vau_t_x(double t, double x){ return (RA * (t + 273.15) * (1.0 + (x / RAV)) / PATM); }
PSICRO_API vau_t_h(double t, double h){ 
    double x = (h - CPAS * t) / (LAMBDA + CPV * t);
    return vau_t_x(t, x); }
PSICRO_API vau_t_tbu(double t, double tbu){ 
    double x = x_t_tbu(t, tbu);
    return vau_t_x(t, x); }
PSICRO_API vau_t_tr(double t, double tr){ 
    double x = x_t_tr(t, tr);
    return vau_t_x(t, x);}
PSICRO_API vau_ur_x(double ur, double x){return vau_t_x(t_ur_x(ur, x), x); }
PSICRO_API vau_ur_h(double ur, double h){return vau_t_h(t_ur_h(ur, h), h); } //cambiata rispetto a .bas
PSICRO_API vau_ur_tbu(double ur, double tbu){return vau_t_x(t_ur_tbu(ur, tbu),x_ur_tbu(ur, tbu) ); }
PSICRO_API vau_ur_tr(double ur, double tr){return vau_ur_x(ur, x_t_ur(tr,100)); }
PSICRO_API vau_x_h(double x, double h){return vau_t_x(t_x_h(x, h), x); }
PSICRO_API vau_x_tbu(double x, double tbu){return vau_t_x( t_x_tbu(x, tbu), x); }
PSICRO_API vau_x_tr(double x, double tr){ (void)x; (void)tr; return 0; }
PSICRO_API vau_h_tbu(double h, double tbu){return vau_t_x( t_h_tbu(h, tbu), x_t_h(h, t_h_tbu(h, tbu)));}
PSICRO_API vau_h_tr(double h, double tr){return vau_x_h(x_t_ur(tr,100), h); }
PSICRO_API vau_tbu_tr(double tbu, double tr){return vau_x_tbu(x_t_ur(tr,100), tbu); }
// --- TARGET 5: BULBO UMIDO (tbu) ---

PSICRO_API tbu_t_ur(double t, double ur) {
    // 1. Calcolo del titolo reale dell'aria allo s
    // * Calcola la temperatura a bulbo umido (tbu) note t e ur.
    // * Utilizza il metodo di Newton-Raphson per trovare lo zero della funzione:
    // * f(tbu) = x_calcolato(t, tbu) - x_reale(t, ur)=0
    double x_target = x_t_ur(t, ur);
    // 2. Stima iniziale (Punto di partenza per Newton-Raphson)
    // Se ur = 100%, tbu = t. Altrimenti tbu < t.
    // Una buona stima iniziale è la temperatura stessa o una via di mezzo tra t e tr.
    double t_curr = t - (t * (100.0 - ur) / 100.0 * 0.5); 
    // 3. Parametri di convergenza
    double epsilon = 0.0001; // Tolleranza in °C
    int max_iter = 50;
    int iter = 0;
    double diff = 1.0;
    double dt = 0.001; // Passo per la derivata numerica
    while (diff > epsilon && iter < max_iter) {
        // Calcolo f(t_curr): differenza tra il titolo calcolato e quello target
        // Usiamo la funzione x_t_tbu che hai già definito nella libreria
        double x_calc = x_t_tbu(t, t_curr);
        double f = x_calc - x_target;
        // Calcolo della derivata numerica df/dtbu
        double x_plus = x_t_tbu(t, t_curr + dt);
        double df_dtbu = (x_plus - x_calc) / dt;
        if (fabs(df_dtbu) < 1e-12) break; // Protezione divisione per zero
        // Aggiornamento Newton-Raphson
        double t_next = t_curr - f / df_dtbu;
        // Limiti fisici: tbu non può superare la temperatura a bulbo asciutto
        if (t_next > t) t_next = t;
    
        diff = fabs(t_next - t_curr);
        t_curr = t_next;
        iter++;
    }
    return t_curr;}
PSICRO_API tbu_t_x(double t, double x){ 
    double ur_calc = ur_t_x(t, x);
    return tbu_t_ur(t, ur_calc); }
PSICRO_API tbu_t_h(double t, double h){ 
    return tbu_t_ur(t, ur_t_h(t, h)); }
PSICRO_API tbu_t_vau(double t, double vau){return tbu_t_ur(t, ur_t_vau(t, vau)); }
PSICRO_API tbu_t_tr(double t, double tr){return tbu_t_x(t, x_t_tr(t, tr));}
PSICRO_API tbu_ur_x(double ur, double x){ 
    double t_calc = t_ur_x(ur, x);
    return tbu_t_ur(t_calc, ur);}
PSICRO_API tbu_ur_h(double ur, double h){ 
    return tbu_t_ur( t_ur_h(ur, h), ur); }
PSICRO_API tbu_ur_vau(double ur, double vau){return tbu_t_ur( t_ur_vau(ur, vau), ur); }
PSICRO_API tbu_ur_tr(double ur, double tr){
    double x_calc = x_t_ur(tr, 100);
    double t_calc = t_ur_x(ur, x_calc);
    return tbu_t_ur(t_calc, ur); }
PSICRO_API tbu_x_h(double x, double h){ 
    double t_calc = t_x_h(x, h);
    double ur_calc = ur_t_x(t_calc, x);
    return tbu_t_ur(t_calc, ur_calc); }
PSICRO_API tbu_x_vau(double x, double vau){ 
    double t_calc = t_x_vau(x, vau);
    double ur_calc = ur_t_x(t_calc, x);
    return tbu_t_ur(t_calc, ur_calc); }
PSICRO_API tbu_x_tr(double x, double tr) {
    (void)x;
    return tr; // In saturazione t = tbu = tr
}
PSICRO_API tbu_h_vau(double h, double vau){
    double t_calc = t_h_vau(h, vau);
    double ur_calc = ur_t_h(t_calc, h);
    return tbu_t_ur(t_calc, ur_calc); }
PSICRO_API tbu_h_tr(double h, double tr){ 
    double x_calc = x_t_ur(tr, 100);
    double t_calc = t_x_h(x_calc, h);
    double ur_calc = ur_t_x(t_calc, x_calc);
    return tbu_t_ur(t_calc, ur_calc); }
PSICRO_API tbu_vau_tr(double vau, double tr){ 
    double x_calc = x_t_ur(tr, 100);
    double t_calc = t_x_vau(x_calc, vau);
    double ur_calc = ur_t_x(t_calc, x_calc);
    return tbu_t_ur(t_calc, ur_calc); }
// --- TARGET 6: PUNTO DI RUGIADA (tr) ---
PSICRO_API tr_t_ur(double t, double ur) {
    // 1. Calcolo il titolo attuale
    double x_attuale = x_t_ur(t, ur);
    
    // 2. Cerco la temperatura che produce quel titolo con ur = 100
    // Usando la tua funzione t_ur_x(ur, x)
    return t_ur_x(100.0, x_attuale);
}
PSICRO_API tr_t_x(double t, double x){ 
    double ur_calc = ur_t_x(t, x);
    return tr_t_ur(t, ur_calc); }
PSICRO_API tr_t_h(double t, double h){
    double ur_calc = ur_t_h(t, h);
    return tr_t_ur(t, ur_calc); }
PSICRO_API tr_t_vau(double t, double vau){ 
    double ur_calc = ur_t_vau(t, vau);
    return tr_t_ur(t, ur_calc); }
PSICRO_API tr_t_tbu(double t, double tbu){ 
    double ur_calc = ur_t_tbu(t, tbu);
    return tr_t_ur(t, ur_calc); }
PSICRO_API tr_ur_x(double ur, double x){ 
    double t_calc = t_ur_x(ur, x);
    double ur_calc = ur_t_x(t_calc, x);
    return tr_t_ur(t_calc, ur_calc); }
PSICRO_API tr_ur_h(double ur, double h){
    double t_calc = t_ur_h(ur, h);
    double ur_calc = ur_t_h(t_calc, h);
    return tr_t_ur(t_calc, ur_calc); }
PSICRO_API tr_ur_vau(double ur, double vau){
    double t_calc = t_ur_vau(ur, vau);
    double ur_calc = ur_t_vau(t_calc, vau);
    return tr_t_ur(t_calc, ur_calc); }
PSICRO_API tr_ur_tbu(double ur, double tbu){ 
    double t_calc = t_ur_tbu(ur, tbu);
    double ur_calc = ur_t_tbu(t_calc, tbu);
    return tr_t_ur(t_calc, ur_calc); }
PSICRO_API tr_x_h(double x, double h){ 
    double t_calc = t_x_h(x, h);
    double ur_calc = ur_t_x(t_calc, x);
    return tr_t_ur(t_calc, ur_calc); }
PSICRO_API tr_x_vau(double x, double vau){
    double t_calc = t_x_vau(x, vau);
    double ur_calc = ur_t_x(t_calc, x);
    return tr_t_ur(t_calc, ur_calc); }
PSICRO_API tr_x_tbu(double x, double tbu){
    double t_calc = t_ur_x(100, x);
    if (t_calc > tbu) t_calc = tbu; // sicurezza
    return t_calc; }
PSICRO_API tr_h_vau(double h, double vau){ 
    double t_calc = t_h_vau(h, vau);
    double ur_calc = ur_t_h(t_calc, h);
    return tr_t_ur(t_calc, ur_calc); }
PSICRO_API tr_h_tbu(double h, double tbu){ 
    double t_calc = t_h_tbu(h, tbu);
    double ur_calc = ur_t_h(t_calc, h);
    return tr_t_ur(t_calc, ur_calc); }
PSICRO_API tr_vau_tbu(double vau, double tbu){
    double t_calc = t_vau_tbu(vau, tbu);
    double ur_calc = ur_t_vau(t_calc, vau);
    return tr_t_ur(t_calc, ur_calc); }

