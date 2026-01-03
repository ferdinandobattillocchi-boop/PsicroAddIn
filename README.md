# PsicroAddIn 

**Excel Add-in for Psychrometric Calculations (C# & C)** *Componente aggiuntivo Excel per calcoli psicrometrici (C# & C)*

PsicroAddIn is a high-performance Microsoft Excel add-in (`.xll`) providing a library for calculating the thermophysical properties of moist air based on **ASHRAE Fundamentals** (Hyland and Wexler equations).

---

## English Version

### Key Features
* **High Performance:** Written in **C** for maximum speed, ideal for massive datasets (e.g., hourly annual simulations).
* **Numerical Precision:** Uses the **Newton-Raphson** algorithm for property inversion.
* **Dynamic Arrays:** Fully supports vectorized inputs (e.g., `A1:A100`) and automatic horizontal spilling.
* **Dual Units:** Supports both **SI** and **IP** (Imperial) units with standard enthalpy zero-point shifts.

### Installation
1. Download the version matching your Excel architecture from the `release` folder:
   * `PsicroAddIn-AddIn64-packed.xll` (for **64-bit** Excel)
   * `PsicroAddIn-AddIn-packed.xll` (for **32-bit** Excel)
2. In Excel, go to **File > Options > Add-ins**.
3. At the bottom, select **Excel Add-ins** in the "Manage" dropdown and click **Go...**.
4. Click **Browse...**, select the `.xll` file, and click **OK**.

### Usage: The PSICRO Function
Main function: `=PSICRO(p1, v1, p2, v2, target, [units])`

* **p1, p2**: Labels of known variables (e.g., `"Tdb"`, `"RH"`, `"H"`).
* **v1, v2**: Numerical values for p1 and p2.
* **target**: Desired output(s) (e.g., `"h"`, `"twb,h"`, or `"all"`).
* **units**: Optional. `"SI"` (default) or `"IP"`.

**Example:** `=PSICRO("T"; 26; "RH"; 50; "H,TWB")`

### Supported Variables
| Code | Description | Units (SI / IP) |
| :--- | :--- | :--- |
| **T / Tdb** | Dry Bulb Temperature | °C / °F |
| **Tbu / Twb** | Wet Bulb Temperature | °C / °F |
| **Tr / Tdp** | Dew Point Temperature | °C / °F |
| **Ur / rh** | Relative Humidity | % |
| **x / w** | Humidity Ratio | kg/kg / lb/lb |
| **h** | Specific Enthalpy | kJ/kg / Btu/lb |
| **vau / v** | Specific Volume | m³/kg / ft³/lb |

---

##  Versione Italiana

### Caratteristiche
* **Performance:** Core in **C** estremamente veloce per calcoli massivi.
* **Precisione:** Algoritmo **Newton-Raphson** per l'inversione delle proprietà.
* **Vettoriale:** Supporta i "Dynamic Arrays" di Excel (es. `A1:A100`).
* **Unità di Misura:** Gestione completa SI e IP (Imperiali) con shift dell'entalpia conforme ASHRAE.

### Installazione
1. Scarica il file dalla cartella `release` in base al tuo Excel:
   * `PsicroAddIn-AddIn64-packed.xll` (**64-bit**)
   * `PsicroAddIn-AddIn-packed.xll` (**32-bit**)
2. In Excel: **File > Opzioni > Componenti aggiuntivi**.
3. In basso, seleziona **Componenti aggiuntivi di Excel** e clicca **Vai...**.
4. Clicca **Sfoglia...**, seleziona il file e conferma.

### Utilizzo
Funzione principale: `=PSICRO(p1; v1; p2; v2; target; [units])`

* **Esempio 1:** `=PSICRO("T"; 26; "UR"; 50; "H")` -> Restituisce l'entalpia.
* **Esempio 2:** `=PSICRO("T"; A1:A20; "UR"; 50; "H")` -> Calcolo vettoriale su 20 celle.
* **Quota:** `=PSICRO.SET.QUOTA(3000)` imposta la pressione atmosferica per i calcoli a 3000m.

### Note Tecniche
* **Equazioni:** Basato sullo standard **ASHRAE Fundamentals 1997** (Hyland & Wexler).
* **Affidabilità:** Libreria core testata dal 2003 e ottimizzata per stabilità numerica.

---

## Acknowledgments & Contributions / Ringraziamenti e Contributi

**EN:** This is a core tool intended for preliminary psychrometric calculations in professional life. I would like to thank anyone who wishes to use, critique, or expand this utility. Please feel free to report any bugs. 
*Note: The 32-bit version is currently untested as I am working on a 64-bit environment.*

**IT:** Questo è uno strumento di base per calcoli psicrometrici di prima approssimazione nella vita professionale. Ringrazio sentitamente chiunque vorrà utilizzarlo, criticarlo, ampliarlo o segnalare eventuali bug.
*Nota: Ad oggi la versione a 32bit non è stata testata personalmente in quanto opero su ambiente a 64bit.*

---

## ⚖️ License
This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)** - see the [LICENSE](LICENSE) file for details.
