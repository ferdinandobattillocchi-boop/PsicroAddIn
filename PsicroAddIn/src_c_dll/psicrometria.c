#include "psicrometria.h"
#include <math.h>
volatile double PATM = 101.325;
void WINAPI set_patm_at_altitude(double altitude) {
    // Patm = 101325 * (1 - 2.25577 * 10^-5 * Quota) ^ 5.2559
    PATM = 101.325 * pow(1.0 - 2.25577e-5 * altitude, 5.2559);
}

// --- FORMULE PSICROMETRICHE ---
PSICRO_API Psat(double t) {
    //Hyland-Wexler Pressione di saturazione in kPa A.M. ASHRAE Fundamentals 2017
    double T = t + 273.15;
    double lnPs;
    if (t >= T_TRIPLO) {
        // ASHRAE Fundamentals - Hyland & Wexler
        double C8 = -5800.2206; //–5.800 220 6 E+03
        double C9 = 1.3914993; //1.391 499 3 E+00
        double C10 = -0.048640239; //–4.864 023 9 E–02
        double C11 = 0.000041764768;//4.176 476 8 E–05
        double C12 = -0.000000014452093;//–1.445 209 3 E–08
        double C13 = 6.5459673; //6.5459673 E+00
        lnPs = (C8 / T) + C9 + (C10 * T) + (C11 * T * T) + (C12 * T * T * T) + (C13 * log(T));
    }
    else {
        // ASHRAE per T < 0 (Ghiaccio)
        double C1 = -5674.5359;//–5.674 535 9 E+03
        double C2 = 6.3925247;//6.392 524 7 E+00
        double C3 = -0.009677843;//–9.677 843 0 E–03
        double C4 = 0.00000062215701;//6.221 570 1 E–07
        double C5 = 0.0000000020747825;//2.074 782 5 E–09
        double C6 = -0.0000000000009484024; //–9.484 024 0 E–13
        double C7 = 4.1635019; //4.163 501 9 E00
        lnPs = (C1 / T) + C2 + (C3 * T) + (C4 * T * T) + (C5 * T * T * T) + (C6 * T * T * T * T) + (C7 * log(T));
    }
    return exp(lnPs) / 1000.0; // Restituisce kPa
}
PSICRO_API dPsat_dt(double t) {
    double T = t + 273.15;
    double ps_kpa = Psat(t); // Otteniamo il valore già diviso per 1000
    double dlnPs;
    if (t >= T_TRIPLO) {
        // Derivata dei termini per Acqua Liquida
        double C8 = -5800.2206;
        double C10 = -0.048640239;
        double C11 = 0.000041764768;
        double C12 = -0.000000014452093;
        double C13 = 6.5459673;
        dlnPs = (-C8 / (T * T)) + C10 + (2.0 * C11 * T) + (3.0 * C12 * T * T) + (C13 / T);
    }
    else {
        // Derivata dei termini per Ghiaccio
        double C1 = -5674.5359;
        double C3 = -0.009677843;
        double C4 = 0.00000062215701;
        double C5 = 0.0000000020747825;
        double C6 = -0.0000000000009484024;
        double C7 = 4.1635019;
        dlnPs = (-C1 / (T * T)) + C3 + (2.0 * C4 * T) + (3.0 * C5 * T * T) + (4.0 * C6 * T * T * T) + (C7 / T);
    }
    // Poiché d(lnPs)/dT = (1/Ps) * dPs/dT
    // Allora dPs/dT = Ps * dlnPs/dT
    // Se ps_kpa è già in kPa, il risultato sarà correttamente in kPa/K
    return ps_kpa * dlnPs;
}
PSICRO_API TPsat(double p_kpa) {
    // 1. GESTIONE LIMITI FISICI
    if (p_kpa <= 0.0001) return -100.0; // Limite inferiore (Ghiaccio profondo)
    if (p_kpa > 20000.0) return 360.0;  // Vicino al punto critico dell'acqu
    // 2. PARAMETRI DI PRECISIONE
    const double eps_p = 1e-7;   // Tolleranza sulla pressione [kPa]
    const double eps_t = 1e-5;   // Tolleranza sulla temperatura [°C]
    const int max_iter = 100;     // Newton converge in 2-4 iterazioni con Magnus
    // 3. STIMA INIZIALE (Magnus-Tetens ottimizzata)
    double t_curr = stima_iniziale_t(p_kpa);
    // 4. CICLO DI NEWTON-RAPHSON
    double t_next = t_curr;
    int iter = 0;
    while (iter < max_iter) {
        // Calcolo Psat e dPsat/dt al valore corrente
        double p_calc = Psat(t_curr);
        double dPdt = dPsat_dt(t_curr);
        // Protezione divisione per zero (teoricamente impossibile qui)
        if (fabs(dPdt) < 1e-15) break;
        // Residuo di pressione (f(x)) e passo (f(x)/f'(x))
        double error_p = p_calc - p_kpa;
        double step = error_p / dPdt;
        t_next = t_curr - step;
        // CRITERIO DI ARRESTO: AND tra precisione P e precisione T
        // x coerenza termodinamica 
        if (fabs(error_p) < eps_p && fabs(step) < eps_t) {
            return t_next;
        }
        // AGGIORNAMENTO
        t_curr = t_next;
        iter++;
    }
    // Se il ciclo termina per max_iter, restituiamo l'ultimo valore calcolato
    return t_next;
}
PSICRO_API stima_iniziale_t(double p_kpa) {
    double a, b, p0;
    //Costanti Ottimizzate (WMO / ASHRAE Lite)
    if (p_kpa >= P_TRIPLO) { // punto triplo per lo switch
        a = 17.62;
        b = 243.12;
        p0 = 0.6112;
    }
    else { // Ghiaccio
        a = 22.46;
        b = 272.62;
        p0 = 0.61115;
    }
    double L = log(p_kpa / p0);
    return (b * L) / (a - L);
}
// --- TITOLO DI SATURAZIONE ALLA TEMPERATURA t ---
PSICRO_API xsat_t(double t) {
    double ps = Psat(t);
    if (ps >= PATM) return 9.999; // Valore di saturazione estremo (quasi 10 kg/kg)
    return (RAV * ps) / (PATM - ps);
}
// --- TARGET 0: TEMPERATURA (t) ---
PSICRO_API t_ur_x(double ur, double x) {
    if (ur <= 0) return -999.0; // Ritorna volutamente un valore non fisico
    double ps = (x * PATM) / ((ur / 100.0) * (RAV + x));
    return TPsat(ps);
}
PSICRO_API t_ur_h(double ur, double h_target) {
    double phi = ur / 100.0;
    int max_iter = 100;
    if (phi <= 0.0) return h_target / CPAS;
    // Stima iniziale
    double t_curr = h_target / (CPAS + phi * 0.05 * LAMBDA);
    const double eps_t = 1e-13;
    for (int i = 0; i < max_iter; i++) {
        double ps = Psat(t_curr);
        double dps = dPsat_dt(t_curr);
        double L_current = LAMBDA;
        double pv = phi * ps;
        double denom = PATM - pv;
        if (denom < 0.001) denom = 0.001;
        double x = (RAV * pv) / denom;
        double dxdt = (RAV * phi * dps * PATM) / (denom * denom);
        // f(t) = h_attuale - h_target
        double f_t = (CPAS * t_curr) + (x * (L_current + CPV * t_curr)) - h_target;
        // f'(t) = dh/dt
        double df_dt = CPAS + dxdt * (L_current + CPV * t_curr) + x * CPV;
        double step = f_t / df_dt;
        // Damping
        if (step > 5.0) step = 5.0;
        if (step < -5.0) step = -5.0;
        t_curr -= step;
        if (fabs(step) < eps_t) return t_curr;
    }
    return t_curr;
}
PSICRO_API t_ur_vau(double ur_percent, double vau_target) {//ok testato
    double phi = ur_percent / 100.0;
    if (phi < 0.0) phi = 0.0;
    if (phi > 1.0) phi = 1.0;
    // 1. STIMA INIZIALE ANALITICA 
    // Usiamo la formula dell'aria secca: T = (P * V) / R
    double t_curr = (PATM * vau_target / RA) - 273.15;
    const double eps_t = 1e-12;
    const int max_iter = 50;
    double t_next = t_curr;
    //2. CICLO DI NEWTON-RAPHSON
    for (int i = 0; i < max_iter; i++) {
        double ps = Psat(t_curr);
        double dps = dPsat_dt(t_curr);
        double T_kelvin = t_curr + 273.15;
        // Funzione obiettivo f(t) derivata dalla legge dei gas
        // f(t) = Ra​*(t+273.15) − vau​⋅(Patm​−phi⋅Psat​(t))=0
        double f_t = (RA * T_kelvin) - vau_target * (PATM - phi * ps);
        // f'(t) = Ra + vau * phi * dPsat/dt
        double df_dt = RA + vau_target * phi * dps;
        if (fabs(df_dt) < 1e-15) break;// Protezione divisione per zero
        double step = f_t / df_dt;
        t_next = t_curr - step;
        if (fabs(step) < eps_t) {
            return t_next;
        }
        t_curr = t_next;
    }
    return t_curr;
}
PSICRO_API t_ur_tbu(double ur, double tbu) {
    if (fabs(ur - 100.0) < 0.00001) return tbu;
    //Algoritmo di bisezione
    double t_low = -5.0;
    double t_high = tbu;//t<=tbu sempre
    double t_mid;
    double f_low, f_high, f_mid, h, hs_bu, hw_bu, x, xs_bu;
    const double eps_t = 1e-6;
    int max_iter = 100;
    int iter = 0;
    if (tbu >= T_TRIPLO) {
        hw_bu = CPW * tbu;
    }
    else {
        hw_bu = CPICE * tbu - LAMBDA_ICE;
        t_high = 0.0;
    }
    xs_bu = xsat_t(tbu);
    hs_bu = h_t_x(tbu, xs_bu);
    while (((t_high - t_low) > eps_t) || (iter < max_iter)) {
        t_mid = 0.5 * (t_low + t_high);
        // trova f(t_low) e f(t_high)
        h = h_t_ur(t_low, ur);
        x = x_t_ur(t_low, ur);
        f_low = h - hs_bu + (xs_bu - x) * hw_bu;
        h = h_t_ur(t_high, ur);
        x = x_t_ur(t_high, ur);
        f_high = h - hs_bu + (xs_bu - x) * hw_bu;
        // espandi l'intervallo se non racchiude lo zero
        if (f_low * f_high > 0) {
            t_low = t_low - 5.0;
            t_high = t_high + 5.0;
            iter++;
            continue;
        }
        else {
            h = h_t_ur(t_mid, ur);
            x = x_t_ur(t_mid, ur);
            f_mid = h - hs_bu + (xs_bu - x) * hw_bu;
            if (fabs(f_mid) < eps_t) {
                return t_mid;
            }
            else {
                // Aggiorna l'intervallo
                if (f_low * f_mid < 0) {
                    t_high = t_mid;
                    f_high = f_mid;
                }
                else {
                    t_low = t_mid;
                    f_low = f_mid;
                }
                iter++;
            }
        }
    }
}
PSICRO_API t_ur_tr(double ur, double tr) { return t_ur_x(ur, x_t_ur(tr, 100)); }
PSICRO_API t_x_h(double x, double h) { return (h - x * LAMBDA) / (CPAS + x * CPV); }//ok analitica
PSICRO_API t_x_vau(double x, double vau) { return vau * (PATM / RA) / (1 + (RV / RA) * x) - 273.15; } //ok analitica  
PSICRO_API t_x_tbu(double x, double tbu) {
    double hs_bu, xs_bu, hw_bu, t;
    xs_bu = xsat_t(tbu);
    hs_bu = h_t_x(tbu, xs_bu);
    if (tbu >= T_TRIPLO) {
        hw_bu = CPW * tbu;
    }
    else {
        hw_bu = CPICE * tbu - LAMBDA_ICE;
    }
    t = (hs_bu - (xs_bu - x) * hw_bu - x * LAMBDA) / (CPAS + x * CPV);
    return t;
}
PSICRO_API t_x_tr(double x, double tr) {
    if ((x <= 0.000001) || (tr <= -273.15)) return -999;
    return tr;
}
PSICRO_API t_vau_tbu(double vau, double tbu) {//da testare 
    double c1, c2, c3, delta, t1, t2, t_final;
    double L_base = LAMBDA;
    double cp_f = CPW;
    double xs = xsat_t(tbu);
    double vau_sat = vau_t_x(tbu, xs);
    if (fabs(vau - vau_sat) < 0.000001) return tbu; // caso di saturazione
    // eguaglio la definizione di vau e Tbu e risolvo il polinomio in t c1*t^2+c2*t+c3=0
    double hwbu = (tbu >= T_TRIPLO) ? (CPW * tbu) : (CPICE * tbu - LAMBDA_ICE);
    double xsbu = xsat_t(tbu);
    double hsbu = h_t_x(tbu, xsbu);
    double k = hsbu - xsbu * hwbu;
    c1 = RA * CPV - RV * CPAS;
    c2 = RA * (LAMBDA - hwbu) + RV * k + 273.15 * (RA * CPV - RV * CPAS) - vau * PATM * CPV;
    c3 = +273.15 * (RA * (LAMBDA - hwbu) + RV * k) - vau * PATM * (LAMBDA - hwbu);
    delta = c2 * c2 - 4.0 * c1 * c3;
    if (delta < 0.0) return NAN;
    t1 = (-c2 + sqrt(delta)) / (2.0 * c1);
    t2 = (-c2 - sqrt(delta)) / (2.0 * c1);
    // La temperatura a bulbo asciutto deve essere >= tbu
    t_final = (t1 >= tbu) ? t1 : t2;
    return t_final;
}// da porre in OFF per pre release
PSICRO_API t_h_vau(double h, double vau) {
    double c1, c2, c3, delta, t1, t2, t_final;
    double L = LAMBDA; // Partiamo con l'ipotesi Liquido
    // Eseguiamo il calcolo analitico
    c1 = CPV - CPAS / RAV;
    c2 = 273.15 * (CPV - CPAS / RAV) + L + h / RAV - vau * CPV * PATM / RA;
    c3 = 273.15 * (L + h / RAV) - L * vau * PATM / RA;
    delta = c2 * c2 - 4.0 * c1 * c3;
    if (delta < 0.0) return NAN;
    t1 = (-c2 + sqrt(delta)) / (2.0 * c1);
    t2 = (-c2 - sqrt(delta)) / (2.0 * c1);
    t_final = (t1 > t2) ? t1 : t2;
    return t_final;
}
PSICRO_API t_h_tbu(double h, double tbu) {
    double x = x_h_tbu(h, tbu);
    return t_x_h(x, h);
}
PSICRO_API t_h_tr(double h, double tr) {
    return t_x_h(x_t_ur(tr, 100), h);
}
PSICRO_API t_vau_tr(double vau, double tr) {
    if ((tr <= -273.15) && (vau >= 0)) return ((vau * PATM / RA) - 273.15);
    return t_x_vau(x_t_ur(tr, 100), vau);
}
PSICRO_API t_tbu_tr(double tbu, double tr) {
    return t_x_tbu(x_t_ur(tr, 100), tbu);
}
// --- TARGET 1: UMIDITÀ RELATIVA (ur) ---
PSICRO_API ur_t_x(double t, double x) {
    if (x <= 0.0) return 0.0;
    double Ps = Psat(t);
    double Pv = (x * PATM) / (RAV + x);
    double ur = (Pv / Ps) * 100.0;
    if (ur >= 100.0) return 100.0;
    if (ur <= 0.0) return 0.0;
    return ur;
}
PSICRO_API ur_t_h(double t, double h) {
    double x = x_t_h(t, h);
    double ur = ur_t_x(t, x);
    if (fabs(ur - 100.0) < 0.000001) return 100.0;
    if (ur <= 0.000001) return 0.0;
    return ur;
}
PSICRO_API ur_t_vau(double t, double vau) {
    double T_kelvin = t + 273.15;
    double x = ((vau * PATM) / (RA * T_kelvin) - 1.0) * RAV;
    double ur = ur_t_x(t, x);
    if (ur >= 100.0) return 100.0;
    if (ur <= 0.0) return 0.0;
    return ur;
}
PSICRO_API ur_t_tbu(double t, double tbu) {
    if (fabs(t - tbu) < 0.000001) return 100;
    double x = x_t_tbu(t, tbu);
    return ur_t_x(t, x);
}
PSICRO_API ur_t_tr(double t, double tr) {
    if (tr >= t) return 100.0;
    if (tr <= -273.15) return 0.0;
    double ur = ur_t_x(t, x_t_ur(tr, 100));
    if (ur >= 100.0) return 100.0;
    if (ur <= 0.0) return 0.0;
    return ur;
}
PSICRO_API ur_x_h(double x, double h) {
    double ur = ur_t_h(t_x_h(x, h), h);
    if (ur >= 100.0) return 100.0;
    if (ur <= 0.0) return 0.0;
    return ur;
}
PSICRO_API ur_x_vau(double x, double vau) {
    double t = (vau * PATM / (RA + RV * x)) - 273.15;
    double ur = ur_t_x(t, x);
    if (ur >= 100.0) return 100.0;
    if (ur <= 0.0) return 0.0;
    return ur;
}
PSICRO_API ur_x_tbu(double x, double tbu) {
    double t = t_x_tbu(x, tbu);
    return ur_t_x(t, x);
}
PSICRO_API ur_x_tr(double x, double tr) {
    (void)x; (void)tr;
    if ((x <= 0.0) || (tr <= -273.15)) return 0.0;
    return -999;
}
PSICRO_API ur_h_vau(double h, double vau) {
    double t = t_h_vau(h, vau);
    double ur = ur_t_h(t, h);
    if (ur >= 100.0) return 100.0;
    if (ur <= 0.0) return 0.0;
    return ur;
}
PSICRO_API ur_h_tbu(double h, double tbu) {
    double t = t_h_tbu(h, tbu);
    double ur = ur_t_h(t, h);
    if (ur >= 100.0) return 100.0;
    if (ur <= 0.0) return 0.0;
    return ur;
}
PSICRO_API ur_h_tr(double h, double tr) {
    if (tr <= -273.15) return 0.0;
    double ur = ur_x_h(x_t_ur(tr, 100), h);
    if (ur >= 100.0) return 100.0;
    if (ur <= 0.0) return 0.0;
    return ur;
}
PSICRO_API ur_vau_tbu(double vau, double tbu) {
    double t = t_vau_tbu(vau, tbu);
    return ur_t_vau(t, vau);
}
PSICRO_API ur_vau_tr(double vau, double tr) {
    if (tr <= -273.15) return 0.0;
    double ur = ur_x_vau(x_t_ur(tr, 100), vau);
    if (ur >= 100.0) return 100.0;
    if (ur <= 0.0) return 0.0;
    return ur;
}
PSICRO_API ur_tbu_tr(double tbu, double tr) {
    double t = t_tbu_tr(tbu, tr);
    return ur_t_tbu(t, tbu);
}
// --- TARGET 2: TITOLO (x) ---
PSICRO_API x_t_ur(double t, double ur) {
    if (ur < 0.000001) return 0.0;
    if (fabs(ur - 100) < 0.000001) {

        // printf("\n xsat_t in xt_ur = %f", xsat_t(t));
        return xsat_t(t);
    }
    double Ps = Psat(t);
    double Pv = (ur / 100.0) * Ps;
    return ((RAV * Pv) / (PATM - Pv));
}
PSICRO_API x_t_h(double t, double h) { return ((h - (CPAS * t)) / (LAMBDA + CPV * t)); }
PSICRO_API x_t_vau(double t, double vau) { return ((vau * PATM) / (RA * (t + 273.15)) - 1.0) * RAV; }
PSICRO_API x_t_tbu(double t, double tbu) {// analitica
    double xs_bu, hs_bu, hw_bu;
    xs_bu = xsat_t(tbu);
    hs_bu = h_t_x(tbu, xs_bu);
    if (tbu >= T_TRIPLO) {
        hw_bu = CPW * tbu;
    }
    else {
        hw_bu = CPICE * tbu - LAMBDA_ICE;
    }
    return ((hs_bu - xs_bu * hw_bu - CPAS * t) / (LAMBDA + CPV * t - hw_bu));//EQ. (33) AFH 2017
}
PSICRO_API x_t_tr(double t, double tr) {
    (void)t;
    double Ps_tr = Psat(tr);
    return (RAV * Ps_tr) / (PATM - Ps_tr);
}
PSICRO_API x_ur_h(double ur, double h) { return x_t_h(t_ur_h(ur, h), h); }
PSICRO_API x_ur_vau(double ur, double vau) { return x_t_ur(t_ur_vau(ur, vau), ur); }
PSICRO_API x_ur_tbu(double ur, double tbu) {
    double t = t_ur_tbu(ur, tbu);
    return x_t_ur(t, ur);
}
PSICRO_API x_ur_tr(double ur, double tr) { return x_t_ur(tr, 100); }
PSICRO_API x_h_vau(double h, double vau) { return x_t_h(t_h_vau(h, vau), h); }
PSICRO_API x_h_tbu(double h, double tbu) {//analitica
    double xs_bu, hs_bu, hw_bu;
    xs_bu = xsat_t(tbu);
    hs_bu = h_t_x(tbu, xs_bu);
    if (tbu >= T_TRIPLO) {
        hw_bu = CPW * tbu;
    }
    else {
        hw_bu = CPICE * tbu - LAMBDA_ICE;
    }
    return (xs_bu - ((hs_bu - h) / (hw_bu)));//EQ. (33) AFH 2017
}
PSICRO_API x_h_tr(double h, double tr) {
    return xsat_t(tr);
}
PSICRO_API x_vau_tbu(double vau, double tbu) {
    double t = t_vau_tbu(vau, tbu);
    return x_t_tbu(t, tbu);
}
PSICRO_API x_vau_tr(double vau, double tr) { return x_t_ur(tr, 100); }
PSICRO_API x_tbu_tr(double tbu, double tr) { return x_t_ur(tr, 100); }
// --- TARGET 3: ENTALPIA (h) --- 
PSICRO_API h_t_ur(double t, double ur) {
    if (ur <= 0.0001) return CPAS * t;
    double x = x_t_ur(t, ur);
    return (CPAS * t) + x * (LAMBDA + CPV * t);
}
PSICRO_API h_t_x(double t, double x) { return (CPAS * t) + x * (LAMBDA + CPV * t); }//ANALITICA
PSICRO_API h_t_vau(double t, double vau) {
    double T_kelvin = t + 273.15;
    double x = ((vau * PATM) / (RA * T_kelvin) - 1.0) * RAV;
    return h_t_x(t, x);
}//ANALITICA
PSICRO_API h_t_tbu(double t, double tbu) {
    double x = x_t_tbu(t, tbu);
    return h_t_x(t, x);
}
PSICRO_API h_t_tr(double t, double tr) {
    if (tr <= -273.15) return CPAS * t;
    double x = x_t_tr(t, tr);
    return h_t_x(t, x);
}
PSICRO_API h_ur_x(double ur, double x) {
    if ((ur <= 0.0001) || (x <= 0.000001)) return -999;
    return h_t_x(t_ur_x(ur, x), x);
}
PSICRO_API h_ur_vau(double ur, double vau) {
    double t = t_ur_vau(ur, vau);
    return h_t_ur(t, ur);
}
PSICRO_API h_ur_tr(double ur, double tr) {
    if ((ur <= 0.0001) || (tr <= -273.15)) return -999;
    return h_ur_x(ur, x_t_ur(tr, 100));
}
PSICRO_API h_ur_tbu(double ur, double tbu) {
    double t = t_ur_tbu(ur, tbu);
    return h_t_ur(t, ur);
}
PSICRO_API h_x_tbu(double x, double tbu) {
    double hs_bu, xs_bu, hw_bu;
    xs_bu = xsat_t(tbu);
    hs_bu = h_t_x(tbu, xs_bu);
    if (tbu >= T_TRIPLO) {
        hw_bu = CPW * tbu;
    }
    else
    {
        hw_bu = CPICE * tbu - LAMBDA_ICE;
    }
    return (hs_bu - (xs_bu - x) * hw_bu);
}
PSICRO_API h_x_tr(double x, double tr) {
    if ((fabs(x - xsat_t(tr))) < 0.000001) return h_t_x(tr, x);
    return 999;
}
PSICRO_API h_x_vau(double x, double vau) {
    //if (x <= 0.000001) return -999;
    double t = (vau * PATM / (RA + RV * x)) - 273.15;
    return h_t_x(t, x);
}
PSICRO_API h_vau_tbu(double vau, double tbu) {
    double t = t_vau_tbu(vau, tbu);
    return h_t_tbu(t, tbu);
}
PSICRO_API h_vau_tr(double vau, double tr) {
    if (tr <= -273.15) {//aria secca 
        double t = t_vau_tr(vau, tr);
        return CPAS * t;
    };
    return h_x_vau(x_t_ur(tr, 100), vau);
}
PSICRO_API h_tbu_tr(double tbu, double tr) {
    double x = x_t_ur(tr, 100);
    return h_x_tbu(x, tbu);
}
// --- TARGET 4: VOLUME SPECIFICO (vau) ---
PSICRO_API vau_t_ur(double t, double ur) {
    double ur2 = ur;
    if (ur <= 0.001) ur2 = 0.0;
    if (ur >= 100.0) ur2 = 100.0;
    return (RA * (t + 273.15) * (1.0 + (x_t_ur(t, ur2) / RAV)) / PATM);
}
PSICRO_API vau_t_x(double t, double x) {
    double x2 = x;
    if (x <= 0.000001) { x2 = 0.0; }
    return (RA * (t + 273.15) * (1.0 + (x2 / RAV)) / PATM);
}
PSICRO_API vau_t_h(double t, double h) {
    double x = x_t_h(t, h);
    return vau_t_x(t, x);
}
PSICRO_API vau_t_tbu(double t, double tbu) {
    double x = x_t_tbu(t, tbu);
    return vau_t_x(t, x);
}
PSICRO_API vau_t_tr(double t, double tr) {
    double x = x_t_tr(t, tr);
    return vau_t_x(t, x);
}
PSICRO_API vau_ur_x(double ur, double x) {
    if (ur <= 0.001 || x <= 0.000001) return 999;
    return vau_t_x(t_ur_x(ur, x), x);
}
PSICRO_API vau_ur_h(double ur, double h) {
    return vau_t_h(t_ur_h(ur, h), h);
} //cambiata rispetto a .bas
PSICRO_API vau_ur_tbu(double ur, double tbu) {
    double t = t_ur_tbu(ur, tbu);
    double x = x_ur_tbu(ur, tbu);
    return vau_t_x(t, x);
}
PSICRO_API vau_ur_tr(double ur, double tr) {
    double ur2 = ur;
    if (ur <= 0.001) ur2 = 0.0;
    if (ur >= 100.0) ur2 = 100.0;
    return vau_ur_x(ur2, x_t_ur(tr, 100));
}
PSICRO_API vau_x_h(double x, double h) {
    double x2 = x;
    if (x <= 0.000001) x2 = 0.0;
    return vau_t_x(t_x_h(x2, h), x2);
}
PSICRO_API vau_x_tbu(double x, double tbu) { return vau_t_x(t_x_tbu(x, tbu), x); }
PSICRO_API vau_x_tr(double x, double tr) { (void)x; (void)tr; return 999; }
PSICRO_API vau_h_tbu(double h, double tbu) {
    double x = x_h_tbu(h, tbu);
    double t = t_h_tbu(h, tbu);
    return vau_t_x(t, x);
}
PSICRO_API vau_h_tr(double h, double tr) {
    double x = x_t_ur(tr, 100);
    double t = t_x_h(x, h);
    return vau_t_x(t, x);
}
PSICRO_API vau_tbu_tr(double tbu, double tr) {
    double x = x_t_ur(tr, 100);
    double t = t_x_tbu(x, tbu);
    return vau_t_x(t, x);
}
// --- TARGET 5: BULBO UMIDO (tbu) ---
static double f_x_h_tbu(double x, double h, double tbu) {
    // funzione obiettivo per il calcolo numerico di tbu
    // f(x,h,tbu)=0 con x e h fissati e tbu icognita
    double t = t_x_h(x, h);
    double xs = xsat_t(tbu);
    double hs_bu = h_t_x(tbu, xs);
    double hw_bu;
    if (tbu >= T_TRIPLO) {
        // Regime di evaporazione (Liquido)
        hw_bu = CPW * tbu;
    }
    else {
        // Regime di sublimazione (Ghiaccio)
        hw_bu = CPICE * tbu - LAMBDA_ICE;
    }
    //eq. bilancio (31) A.F.H. 2017
    return h + (xs - x) * hw_bu - hs_bu;
}
PSICRO_API tbu_x_h(double x, double h) {
    //Applica il metodo della bisezione
    int max_iter = 200;
    const double tbu_low_min = -110.0;
    const double tbu_high_max = 180.0;
    const double eps = 1e-8;
    double t = t_x_h(x, h);
    double xs = xsat_t(t);
    //a saturazione ritorna t
   // if (fabs(xs - x) <= eps) {return t; }
    double tbu_low = ((-(h / CPAS) - 5.0) < tbu_low_min) ? (-(h / CPAS) - 5.0) : tbu_low_min / 2;//innesca la bisezione anche con h=0
    double tbu_high = ((h / CPAS) < tbu_high_max) ? (h / CPAS) : tbu_high_max / 2;
    if (h < 0.0) {
        double temp = tbu_high;
        tbu_high = tbu_low;
        tbu_low = temp;
    }
    double f_low = f_x_h_tbu(x, h, tbu_low);
    double f_high = f_x_h_tbu(x, h, tbu_high);
    // Regola l'intervallo se i segni sono uguali
    if ((f_low * f_high > 0.0)) {
        while (f_low * f_high > 0.0) {
            tbu_high += 5.0;
            tbu_low -= 5.0;
            f_low = f_x_h_tbu(x, h, tbu_low);
            f_high = f_x_h_tbu(x, h, tbu_high);
            if ((tbu_high > tbu_high_max) || (tbu_low < tbu_low_min)) {
                //          printf("Bisezione fallita\n");
                return -999; // nessuna soluzione
            }
        }
    }
    for (int iter = 0; iter < max_iter; iter++) {
        double tbu_mid = (tbu_low + tbu_high) / 2.0;
        double f_mid = f_x_h_tbu(x, h, tbu_mid);
        if (fabs(f_mid) < eps) {
            //  printf("\n--- Iter: %d --- \n",iter);
            return tbu_mid;
        }
        if (f_low * f_mid < 0.0) {
            tbu_high = tbu_mid;
            f_high = f_mid;
        }
        else {
            tbu_low = tbu_mid;
            f_low = f_mid;
        }
    }
    return (tbu_low + tbu_high) / 2.0;
}//<- funzione master
PSICRO_API tbu_t_ur(double t, double ur) {
    if (fabs(ur - 100.0) <= 0.00001) return t; // sicurezza
    double x = x_t_ur(t, ur);
    double h = h_t_ur(t, ur);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_t_x(double t, double x) {
    double h = h_t_x(t, x);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_t_h(double t, double h) {
    double x = x_t_h(t, h);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_t_vau(double t, double vau) {
    double x = x_t_vau(t, vau);
    double h = h_t_vau(t, vau);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_t_tr(double t, double tr) {
    if (fabs(t - tr) < 0.000001) return t;
    double x = x_t_tr(t, tr);
    double h = h_t_x(t, x);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_ur_x(double ur, double x) {
    if (fabs(ur - 100.0) <= 0.00001) t_ur_x(100, x);
    double h = h_ur_x(ur, x);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_ur_h(double ur, double h) {
    double x = x_ur_h(ur, h);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_ur_vau(double ur, double vau) {
    double x = x_ur_vau(ur, vau);
    double h = h_ur_vau(ur, vau);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_ur_tr(double ur, double tr) {
    if (fabs(ur - 100.0) < 0.000001) return tr;
    double x_calc = x_ur_tr(ur, tr);
    double h_calc = h_ur_x(ur, x_calc);
    return tbu_x_h(x_calc, h_calc);
}
PSICRO_API tbu_x_vau(double x, double vau) {
    double h = h_x_vau(x, vau);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_x_tr(double x, double tr) {
    (void)x;
    return 999;
} // In saturazione t = tbu = tr}
PSICRO_API tbu_h_vau(double h, double vau) {
    double x = x_h_vau(h, vau);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_h_tr(double h, double tr) {
    double x = x_h_tr(h, tr);
    return tbu_x_h(x, h);
}
PSICRO_API tbu_vau_tr(double vau, double tr) {
    double x = x_vau_tr(vau, tr);
    double h = h_x_vau(x, vau);
    return tbu_x_h(x, h);
}

// --- TARGET 6: PUNTO DI RUGIADA (tr) ---
PSICRO_API tr_t_ur(double t, double ur) {
    // 1. Calcolo il titolo attuale
    if (ur <= 0.001) return -273.15;
    if (fabs(ur - 100.0) < 0.00001) return t;
    double x_attuale = x_t_ur(t, ur);
    // 2. Cerco la temperatura che produce quel titolo con ur = 100
    // Usando la tua funzione t_ur_x(ur, x)
    return t_ur_x(100.0, x_attuale);
}
PSICRO_API tr_t_x(double t, double x) {
    if (x <= 0.0) return -273.15;
    if (x >= xsat_t(t)) return t; // Saturazione
    double ur_calc = ur_t_x(t, x);
    return tr_t_ur(t, ur_calc);
}
PSICRO_API tr_t_h(double t, double h) {
    double ur_calc = ur_t_h(t, h);
    return tr_t_ur(t, ur_calc);
}
PSICRO_API tr_t_vau(double t, double vau) {
    double ur_calc = ur_t_vau(t, vau);
    return tr_t_ur(t, ur_calc);
}
PSICRO_API tr_t_tbu(double t, double tbu) {
    if (fabs(t - tbu) <= 0.000001) return tbu; // sicurezza
    double x = x_t_tbu(t, tbu);
    return tr_t_x(t, x);
}
PSICRO_API tr_ur_x(double ur, double x) {
    if ((x <= 0.0) || (ur <= 0.001)) return -273.15;
    if (ur >= 100.0) return t_ur_x(100, x); // Saturazione
    double t_calc = t_ur_x(ur, x);
    double ur_calc = ur_t_x(t_calc, x);
    return tr_t_ur(t_calc, ur_calc);
}
PSICRO_API tr_ur_h(double ur, double h) {
    if (ur <= 0.001) return -273.15;
    if (ur >= 100.0) return t_ur_h(100, h); // Saturazione
    double t_calc = t_ur_h(ur, h);
    double ur_calc = ur_t_h(t_calc, h);
    return tr_t_ur(t_calc, ur_calc);
}
PSICRO_API tr_ur_vau(double ur, double vau) {
    if (ur <= 0.001) return -273.15;
    if (ur >= 100.0) return t_ur_vau(100, vau); // Saturazione
    double t_calc = t_ur_vau(ur, vau);
    double ur_calc = ur_t_vau(t_calc, vau);
    return tr_t_ur(t_calc, ur_calc);
}
PSICRO_API tr_ur_tbu(double ur, double tbu) {
    double t_calc = t_ur_tbu(ur, tbu);
    double ur_calc = ur_t_tbu(t_calc, tbu);
    return tr_t_ur(t_calc, ur_calc);
}
PSICRO_API tr_x_h(double x, double h) {
    if (x < 0.0) return -273.15;
    double t = t_x_h(x, h);
    double xsat = xsat_t(t);
    if (fabs(x - xsat) <= 0.000001) return t_ur_x(100, x); // Saturazione
    double t_calc = t_x_h(x, h);
    double ur_calc = ur_t_x(t_calc, x);
    return tr_t_ur(t_calc, ur_calc);
}
PSICRO_API tr_x_vau(double x, double vau) {
    double t_calc = t_x_vau(x, vau);
    double ur_calc = ur_t_x(t_calc, x);
    return tr_t_ur(t_calc, ur_calc);
}
PSICRO_API tr_x_tbu(double x, double tbu) {
    double t_calc = t_x_tbu(x, tbu);
    if (x < 0.000001) return -273.15;
    if (fabs(t_calc - tbu) >= 0.000001) t_calc = tbu; // sicurezza
    return tr_t_x(t_calc, x);
}
PSICRO_API tr_h_vau(double h, double vau) {
    double t_calc = t_h_vau(h, vau);
    double ur_calc = ur_t_h(t_calc, h);
    return tr_t_ur(t_calc, ur_calc);
}
PSICRO_API tr_h_tbu(double h, double tbu) {
    double t_calc = t_h_tbu(h, tbu);
    double ur_calc = ur_t_h(t_calc, h);
    return tr_t_ur(t_calc, ur_calc);
}
PSICRO_API tr_vau_tbu(double vau, double tbu) {
    double t_calc = t_vau_tbu(vau, tbu);
    double ur_calc = ur_t_vau(t_calc, vau);
    return tr_t_ur(t_calc, ur_calc);
}
