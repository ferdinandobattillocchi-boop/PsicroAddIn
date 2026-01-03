/*
 * PsicroAddIn - Libreria Psicrometrica per Excel
 * Copyright (C) 2024  Ferdinando Battillocchi
 * * Questo programma è software libero: è possibile ridistribuirlo e/o 
 * modificarlo secondo i termini della licenza GNU General Public 
 * License come pubblicata dalla Free Software Foundation, versione 3.
 * * Questo programma è distribuito nella speranza che possa essere utile, 
 * ma SENZA ALCUNA GARANZIA; senza neppure la garanzia implicita di 
 * COMMERCIABILITÀ o IDONEITÀ PER UNO SCOPO PARTICOLARE. 
 * Vedere la licenza GNU General Public License per ulteriori dettagli.
 * * Dovresti aver ricevuto una copia della licenza GNU General Public License 
 * insieme a questo programma. In caso contrario, vedi <https://www.gnu.org/licenses/>.
 */
using ExcelDna.Integration;
using ExcelDna.Integration.CustomUI;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Remoting.Messaging;
using System.Text;
using System.Windows.Forms; // <- aggiungi questa using in cima al file
using System.Xml.Linq;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;


public class PsicroAddIn : IExcelAddIn
{
    //const string DLL_PATH = @"C:\Users\Utente\source\repos\psicro\x64\Debug\psicro.dll";
    
    const string DLL_PATH = "psicro.dll";
    
    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    static extern bool SetDllDirectory(string lpPathName);

    [DllImport("kernel32.dll", SetLastError = true)]
    static extern IntPtr LoadLibrary(string lpFileName);

    public void AutoOpen()
    {
        try
        {
            var asm = Assembly.GetExecutingAssembly();

            // 1. Trova la risorsa embedded psicro.dll
            string resourceName = null;
            foreach (string n in asm.GetManifestResourceNames())
            {
                if (n.ToLower().Contains("psicro"))
                {
                    resourceName = n;
                    break;
                }
            }

            if (resourceName == null)
                throw new Exception("Risorsa psicro.dll non trovata");

            // 2. Usa AppData invece di TEMP (più persistente e pulito)
            string appDataDir = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            string psicroDir = Path.Combine(appDataDir, "PsicroAddIn");
            Directory.CreateDirectory(psicroDir);

            string dllPath = Path.Combine(psicroDir, "psicro.dll");
            string versionPath = Path.Combine(psicroDir, "version.txt");

            // 3. Verifica se dobbiamo copiare (solo se nuova versione)
            string currentVersion = asm.GetName().Version.ToString();
            bool needCopy = true;

            if (File.Exists(dllPath) && File.Exists(versionPath))
            {
                string existingVersion = File.ReadAllText(versionPath);
                if (existingVersion == currentVersion)
                {
                    // Versione uguale, non serve copiare
                    needCopy = false;
                }
            }

            // 4. Copia la DLL solo se necessario
            if (needCopy)
            {
                using (Stream s = asm.GetManifestResourceStream(resourceName))
                using (FileStream fs = File.Create(dllPath))
                {
                    s.CopyTo(fs);
                }

                // Salva la versione corrente
                File.WriteAllText(versionPath, currentVersion);
            }

            // 5. Imposta la directory e carica la DLL
            SetDllDirectory(psicroDir);

            IntPtr handle = LoadLibrary(dllPath);

            if (handle == IntPtr.Zero)
            {
                int error = Marshal.GetLastWin32Error();
                throw new Exception($"Errore caricamento DLL: {error}\nPercorso: {dllPath}");
            }

            // Successo - puoi rimuovere questo messaggio se vuoi
            // MessageBox.Show($"DLL caricata da:\n{dllPath}", "PsicroAddIn - Successo");
        }
        catch (Exception ex)
        {
            MessageBox.Show(ex.ToString(), "PsicroAddIn");
        }
    }
    public void AutoClose() { }



    #region 1. DICHIARAZIONI DLL C (Import)


    // Carichiamo la funzione per la quota dalla tua DLL C
    [DllImport("psicro.dll", CallingConvention = CallingConvention.StdCall)]public static extern double Excel_set_quota(double altitude);
    //[DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_get_patm_at_altitude(double altitude);
    
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_Psat(double t);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_TPsat(double p_kpa);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_xsat_t(double t);

    // T - Temperatura
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_ur_x(double ur, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_ur_h(double ur, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_ur_vau(double ur, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_ur_tbu(double ur, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_ur_tr(double ur, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_x_h(double x, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_x_vau(double x, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_x_tbu(double x, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_x_tr(double x, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_h_vau(double h, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_h_tbu(double h, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_h_tr(double h, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_vau_tbu(double vau, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_vau_tr(double vau, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_t_tbu_tr(double tbu, double tr);

    // UR - Umidità Relativa
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_t_x(double t, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_t_h(double t, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_t_vau(double t, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_t_tbu(double t, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_t_tr(double t, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_x_h(double x, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_x_vau(double x, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_x_tbu(double x, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_x_tr(double x, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_h_vau(double h, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_h_tbu(double h, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_h_tr(double h, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_vau_tbu(double vau, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_vau_tr(double vau, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_ur_tbu_tr(double tbu, double tr);

    // X - Titolo (Umidità specifica)
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_t_ur(double t, double ur);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_t_h(double t, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_t_vau(double t, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_t_tbu(double t, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_t_tr(double t, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_ur_h(double ur, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_ur_vau(double ur, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_ur_tbu(double ur, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_ur_tr(double ur, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_h_vau(double h, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_h_tbu(double h, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_h_tr(double h, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_vau_tbu(double vau, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_vau_tr(double vau, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_x_tbu_tr(double tbu, double tr);

    // H - Entalpia
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_t_ur(double t, double ur);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_t_x(double t, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_t_vau(double t, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_t_tbu(double t, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_t_tr(double t, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_ur_x(double ur, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_ur_vau(double ur, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_ur_tbu(double ur, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_ur_tr(double ur, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_x_vau(double x, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_x_tbu(double x, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_x_tr(double x, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_vau_tbu(double vau, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_vau_tr(double vau, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_h_tbu_tr(double tbu, double tr);

    // VAU - Volume specifico
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_t_ur(double t, double ur);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_t_x(double t, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_t_h(double t, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_t_tbu(double t, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_t_tr(double t, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_ur_x(double ur, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_ur_h(double ur, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_ur_tbu(double ur, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_ur_tr(double ur, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_x_h(double x, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_x_tbu(double x, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_x_tr(double x, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_h_tbu(double h, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_h_tr(double h, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_vau_tbu_tr(double tbu, double tr);

    // TBU - Bulbo umido
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_t_ur(double t, double ur);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_t_x(double t, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_t_h(double t, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_t_vau(double t, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_t_tr(double t, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_ur_x(double ur, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_ur_h(double ur, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_ur_vau(double ur, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_ur_tr(double ur, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_x_h(double x, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_x_vau(double x, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_x_tr(double x, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_h_vau(double h, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_h_tr(double h, double tr);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tbu_vau_tr(double vau, double tr);

    // TR - Punto di rugiada
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_t_ur(double t, double ur);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_t_x(double t, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_t_h(double t, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_t_vau(double t, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_t_tbu(double t, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_ur_x(double ur, double x);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_ur_h(double ur, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_ur_vau(double ur, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_ur_tbu(double ur, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_x_h(double x, double h);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_x_vau(double x, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_x_tbu(double x, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_h_vau(double h, double vau);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_h_tbu(double h, double tbu);
    [DllImport(DLL_PATH, CallingConvention = CallingConvention.StdCall)] public static extern double Excel_tr_vau_tbu(double vau, double tbu);
    
    #endregion

    #region 2. FUNZIONI EXCEL (Esposizione)

    private const string CAT = "Psicrometria";
  

  


[ExcelFunction(Name = "PSICRO",Description = "Calcolo psicrometrico / Psychrometric calculation",Category = CAT)]
    public static object PsicroProp(
    [ExcelArgument(Description = "Primo input (t, ur, x, h, v...) / First input")] string p1,
    [ExcelArgument(Description = "Valore 1 / Value 1")] object val1,
    [ExcelArgument(Description = "Secondo input / Second input")] string p2,
    [ExcelArgument(Description = "Valore 2 / Value 2")] object val2,
    [ExcelArgument(Description = "Proprietà target (es. 'h' o 'tutto') / Target property")] string target,
    [ExcelArgument(Description = "Unità (SI default, IP) / Unit")] object unit)
    {
        try
        {
            // 1. Normalizzazione Input (Gestisce Range o valori singoli)
            // Dobbiamo farlo subito per sapere quante righe (totalRows) elaborare
            object[,] mat1 = ToMatrix(val1);
            object[,] mat2 = ToMatrix(val2);

            int rows1 = mat1.GetLength(0);
            int rows2 = mat2.GetLength(0);
            int totalRows = Math.Max(rows1, rows2);

            // 2. Parsing dei Target e indici
            int id1 = GetPropIndex(p1);
            int id2 = GetPropIndex(p2);
            List<int> targetIndices = new List<int>();

            if (string.IsNullOrEmpty(target) || target.ToLower() == "tutto" || target.ToLower() == "all")
            {
                for (int i = 0; i <= 6; i++)
                {
                    if (i != id1 && i != id2) targetIndices.Add(i);
                }
            }
            else
            {
                string[] tNames = target.ToLower().Split(new[] { ',', ';', ' ' }, StringSplitOptions.RemoveEmptyEntries);
                foreach (var name in tNames)
                {
                    int idx = GetPropIndex(name);
                    if (idx != -1) targetIndices.Add(idx);
                }
            }

            // 3. Preparazione Output
            int outCols = targetIndices.Count;
            object[,] output = new object[totalRows, outCols];

            // Determiniamo se siamo in modalità IP
            bool isIP = unit?.ToString().ToUpper() == "IP";

            // 4. Ciclo di Calcolo
            for (int r = 0; r < totalRows; r++)
            {
                double v1 = ExtractDouble(mat1, r);
                double v2 = ExtractDouble(mat2, r);

                // --- CONVERSIONE INGRESSO (IP -> SI) ---
                if (isIP)
                {
                    v1 = ConvertUnit(v1, id1, true);
                    v2 = ConvertUnit(v2, id2, true);
                }

                for (int c = 0; c < outCols; c++)
                {
                    int currentTargetIdx = targetIndices[c];

                    // Passiamo l'indice del target corrente
                    object res = EseguiSwitchCalcolo(currentTargetIdx, id1, id2, v1, v2);

                    // --- CONVERSIONE USCITA (SI -> IP) ---
                    if (isIP && res is double d)
                    {
                        output[r, c] = ConvertUnit(d, currentTargetIdx, false);
                    }
                    else
                    {
                        output[r, c] = res;
                    }
                }
            }

            return output;
        }
        catch (Exception ex)
        {
            return "Errore: " + ex.Message;
        }
    }

    private static int GetPropIndex(string p)
    {
        if (string.IsNullOrEmpty(p)) return -1;
        switch (p.Trim().ToLower())
        {
            case "t": case "tdb": return 0;
            case "ur": case "rh": return 1;
            case "x": case "w": return 2;
            case "h": return 3;
            case "vau": case "v": return 4;
            case "tbu": case "twb": return 5;
            case "tr": case "tdp": return 6;
            default: return -1;
        }
    }

    
    private static object EseguiSwitchCalcolo(int idT, int id1, int id2, double val1, double val2)
    {
        if (idT == -1 || id1 == -1 || id2 == -1) return "#NOME_ERR#";
        // Logica di Swap: i1 deve essere sempre il minore
        int i1, i2;
        double v1, v2;
        if (id1 < id2) { i1 = id1; i2 = id2; v1 = val1; v2 = val2; }
        else { i1 = id2; i2 = id1; v1 = val2; v2 = val1; }

        int pairID = (i1 * 10) + i2;
        double ris = 0;

        switch (idT)
        {
            case 0: // TARGET t
                if (pairID == 12) ris = Excel_t_ur_x(v1, v2);
                else if (pairID == 13) ris = Excel_t_ur_h(v1, v2);
                else if (pairID == 14) ris = Excel_t_ur_vau(v1, v2);
                else if (pairID == 15) ris = Excel_t_ur_tbu(v1, v2);
                else if (pairID == 16) ris = Excel_t_ur_tr(v1, v2);
                else if (pairID == 23) ris = Excel_t_x_h(v1, v2);
                else if (pairID == 24) ris = Excel_t_x_vau(v1, v2);
                else if (pairID == 25) ris = Excel_t_x_tbu(v1, v2);
                else if (pairID == 26) ris = Excel_t_x_tr(v1, v2);
                else if (pairID == 34) ris = Excel_t_h_vau(v1, v2);
                else if (pairID == 35) ris = Excel_t_h_tbu(v1, v2);
                else if (pairID == 36) ris = Excel_t_h_tr(v1, v2);
                else if (pairID == 45) ris = Excel_t_vau_tbu(v1, v2);
                else if (pairID == 46) ris = Excel_t_vau_tr(v1, v2);
                else if (pairID == 56) ris = Excel_t_tbu_tr(v1, v2);
                else return "#N/D";
                break;

            case 1: // TARGET ur
                if (pairID == 02) ris = Excel_ur_t_x(v1, v2);
                else if (pairID == 03) ris = Excel_ur_t_h(v1, v2);
                else if (pairID == 04) ris = Excel_ur_t_vau(v1, v2);
                else if (pairID == 05) ris = Excel_ur_t_tbu(v1, v2);
                else if (pairID == 06) ris = Excel_ur_t_tr(v1, v2);
                else if (pairID == 23) ris = Excel_ur_x_h(v1, v2);
                else if (pairID == 24) ris = Excel_ur_x_vau(v1, v2);
                else if (pairID == 25) ris = Excel_ur_x_tbu(v1, v2);
                else if (pairID == 26) ris = Excel_ur_x_tr(v1, v2);
                else if (pairID == 34) ris = Excel_ur_h_vau(v1, v2);
                else if (pairID == 35) ris = Excel_ur_h_tbu(v1, v2);
                else if (pairID == 36) ris = Excel_ur_h_tr(v1, v2);
                else if (pairID == 45) ris = Excel_ur_vau_tbu(v1, v2);
                else if (pairID == 46) ris = Excel_ur_vau_tr(v1, v2);
                else if (pairID == 56) ris = Excel_ur_tbu_tr(v1, v2);
                else return "#N/D";
                break;

            case 2: // TARGET x
                if (pairID == 01) ris = Excel_x_t_ur(v1, v2);
                else if (pairID == 03) ris = Excel_x_t_h(v1, v2);
                else if (pairID == 04) ris = Excel_x_t_vau(v1, v2);
                else if (pairID == 05) ris = Excel_x_t_tbu(v1, v2);
                else if (pairID == 06) ris = Excel_x_t_tr(v1, v2);
                else if (pairID == 13) ris = Excel_x_ur_h(v1, v2);
                else if (pairID == 14) ris = Excel_x_ur_vau(v1, v2);
                else if (pairID == 15) ris = Excel_x_ur_tbu(v1, v2);
                else if (pairID == 16) ris = Excel_x_ur_tr(v1, v2);
                else if (pairID == 34) ris = Excel_x_h_vau(v1, v2);
                else if (pairID == 35) ris = Excel_x_h_tbu(v1, v2);
                else if (pairID == 36) ris = Excel_x_h_tr(v1, v2);
                else if (pairID == 45) ris = Excel_x_vau_tbu(v1, v2);
                else if (pairID == 46) ris = Excel_x_vau_tr(v1, v2);
                else if (pairID == 56) ris = Excel_x_tbu_tr(v1, v2);
                else return "#N/D";
                break;

            case 3: // TARGET h
                if (pairID == 01) ris = Excel_h_t_ur(v1, v2);
                else if (pairID == 02) ris = Excel_h_t_x(v1, v2);
                else if (pairID == 04) ris = Excel_h_t_vau(v1, v2);
                else if (pairID == 05) ris = Excel_h_t_tbu(v1, v2);
                else if (pairID == 06) ris = Excel_h_t_tr(v1, v2);
                else if (pairID == 12) ris = Excel_h_ur_x(v1, v2);
                else if (pairID == 14) ris = Excel_h_ur_vau(v1, v2);
                else if (pairID == 15) ris = Excel_h_ur_tbu(v1, v2);
                else if (pairID == 16) ris = Excel_h_ur_tr(v1, v2);
                else if (pairID == 24) ris = Excel_h_x_vau(v1, v2);
                else if (pairID == 25) ris = Excel_h_x_tbu(v1, v2);
                else if (pairID == 26) ris = Excel_h_x_tr(v1, v2);
                else if (pairID == 45) ris = Excel_h_vau_tbu(v1, v2);
                else if (pairID == 46) ris = Excel_h_vau_tr(v1, v2);
                else if (pairID == 56) ris = Excel_h_tbu_tr(v1, v2);
                else return "#N/D";
                break;

            case 4: // TARGET vau
                if (pairID == 01) ris = Excel_vau_t_ur(v1, v2);
                else if (pairID == 02) ris = Excel_vau_t_x(v1, v2);
                else if (pairID == 03) ris = Excel_vau_t_h(v1, v2);
                else if (pairID == 05) ris = Excel_vau_t_tbu(v1, v2);
                else if (pairID == 06) ris = Excel_vau_t_tr(v1, v2);
                else if (pairID == 12) ris = Excel_vau_ur_x(v1, v2);
                else if (pairID == 13) ris = Excel_vau_ur_h(v1, v2);
                else if (pairID == 15) ris = Excel_vau_ur_tbu(v1, v2);
                else if (pairID == 16) ris = Excel_vau_ur_tr(v1, v2);
                else if (pairID == 23) ris = Excel_vau_x_h(v1, v2);
                else if (pairID == 25) ris = Excel_vau_x_tbu(v1, v2);
                else if (pairID == 26) ris = Excel_vau_x_tr(v1, v2);
                else if (pairID == 35) ris = Excel_vau_h_tbu(v1, v2);
                else if (pairID == 36) ris = Excel_vau_h_tr(v1, v2);
                else if (pairID == 56) ris = Excel_vau_tbu_tr(v1, v2);
                else return "#N/D";
                break;

            case 5: // TARGET tbu
                if (pairID == 01) ris = Excel_tbu_t_ur(v1, v2);
                else if (pairID == 02) ris = Excel_tbu_t_x(v1, v2);
                else if (pairID == 03) ris = Excel_tbu_t_h(v1, v2);
                else if (pairID == 04) ris = Excel_tbu_t_vau(v1, v2);
                else if (pairID == 06) ris = Excel_tbu_t_tr(v1, v2);
                else if (pairID == 12) ris = Excel_tbu_ur_x(v1, v2);
                else if (pairID == 13) ris = Excel_tbu_ur_h(v1, v2);
                else if (pairID == 14) ris = Excel_tbu_ur_vau(v1, v2);
                else if (pairID == 16) ris = Excel_tbu_ur_tr(v1, v2);
                else if (pairID == 23) ris = Excel_tbu_x_h(v1, v2);
                else if (pairID == 24) ris = Excel_tbu_x_vau(v1, v2);
                else if (pairID == 26) ris = Excel_tbu_x_tr(v1, v2);
                else if (pairID == 34) ris = Excel_tbu_h_vau(v1, v2);
                else if (pairID == 36) ris = Excel_tbu_h_tr(v1, v2);
                else if (pairID == 46) ris = Excel_tbu_vau_tr(v1, v2);
                else return "#N/D";
                break;

            case 6: // TARGET tr
                if (pairID == 01) ris = Excel_tr_t_ur(v1, v2);
                else if (pairID == 02) ris = Excel_tr_t_x(v1, v2);
                else if (pairID == 03) ris = Excel_tr_t_h(v1, v2);
                else if (pairID == 04) ris = Excel_tr_t_vau(v1, v2);
                else if (pairID == 05) ris = Excel_tr_t_tbu(v1, v2);
                else if (pairID == 12) ris = Excel_tr_ur_x(v1, v2);
                else if (pairID == 13) ris = Excel_tr_ur_h(v1, v2);
                else if (pairID == 14) ris = Excel_tr_ur_vau(v1, v2);
                else if (pairID == 15) ris = Excel_tr_ur_tbu(v1, v2);
                else if (pairID == 23) ris = Excel_tr_x_h(v1, v2);
                else if (pairID == 24) ris = Excel_tr_x_vau(v1, v2);
                else if (pairID == 25) ris = Excel_tr_x_tbu(v1, v2);
                else if (pairID == 34) ris = Excel_tr_h_vau(v1, v2);
                else if (pairID == 35) ris = Excel_tr_h_tbu(v1, v2);
                else if (pairID == 45) ris = Excel_tr_vau_tbu(v1, v2);
                else return "#N/D";
                break;

            default: return "#N/D";
        }
        return ris;
    }

    private static object[,] ToMatrix(object input)
    {
        // Se è già una matrice di oggetti (caso standard Excel-DNA per i Range)
        if (input is object[,] matrix) return matrix;

        // Se Excel passa un riferimento (ExcelReference), estraiamo il valore
        if (input is ExcelDna.Integration.ExcelReference exRef)
        {
            object val = exRef.GetValue();
            if (val is object[,] m) return m;
            return new object[,] { { val } };
        }

        // Se è un valore singolo (double, string, ecc.) lo mettiamo in una matrice 1x1
        return new object[,] { { input } };
    }

    private static double ExtractDouble(object[,] matrix, int r)
    {
        int maxRows = matrix.GetLength(0);
        int maxCols = matrix.GetLength(1);

        // LOGICA DI BROADCASTING:
        // Se il range ha più righe, prendi la riga 'r'.
        // Se ne ha una sola (o è un valore singolo), usa sempre la riga 0.
        int row = (maxRows > 1) ? Math.Min(r, maxRows - 1) : 0;

        // Gestione range orizzontali (raro, ma meglio averla)
        int col = (maxCols > 1 && maxRows == 1) ? Math.Min(r, maxCols - 1) : 0;

        object val = matrix[row, col];

        if (val is double d) return d;

        // Gestione numeri scritti come testo o celle vuote
        if (val != null)
        {
            string s = val.ToString().Replace(',', '.'); // Forza il punto come decimale
            if (double.TryParse(s, System.Globalization.NumberStyles.Any, System.Globalization.CultureInfo.InvariantCulture, out double res))
                return res;
        }

        return 0.0;
    }

    private static double ConvertUnit(double value, int propIdx, bool toSI)
    {
        // Se il valore non è un numero valido, lo restituiamo così com'è
        if (double.IsNaN(value)) return value;

        switch (propIdx)
        {
            case 0: // t (Temperatura Secca)
            case 5: // tbu (Bulbo Umido)
            case 6: // tr (Punto di Rugiada)
                    // IP (°F) <-> SI (°C)
                return toSI ? (value - 32.0) / 1.8 : (value * 1.8) + 32.0;

            case 1: // ur (Umidità Relativa)
                    // Solitamente espressa in % in entrambi i sistemi
                return value;

            case 2: // x (Titolo di umidità)
                    // IP (lb_w/lb_da) <-> SI (kg_w/kg_da) -> Il rapporto è identico
                    // Nota: Se usassi i "Grains", dovresti dividere/moltiplicare per 7000
                return value;

            case 3: // h (Entalpia specifica)
                    // Offset ASHRAE per far corrispondere 0°F aria secca a 0 BTU/lb
                    // In SI, a -17.77°C (0°F), h è circa -17.89 kJ/kg
                double hOffsetASHRAE = 17.88444668;
                if (toSI)
                {
                    // Riportiamo lo zero da 0°F a 0°C prima di darlo alla DLL
                    return (value * 2.326) - hOffsetASHRAE;
                }
                else
                {
                    // Spostiamo lo zero da 0°C a 0°F prima di mostrarlo all'utente
                    return (value + hOffsetASHRAE) / 2.326;
                }
 
            case 4: // vau (Volume specifico)
                    // IP (ft³/lb) <-> SI (m³/kg) -> 1 ft³/lb = 0.062428 m³/kg
                return toSI ? value * 0.062428 : value / 0.062428;

            default:
                return value;
        }
    }


    [ExcelFunction( Name = "PSICRO.SET.QUOTA", Description = "Imposta la quota e aggiorna la pressione globale / Set altitude and update pressure", Category = "Psicrometria", IsVolatile = true)]
    public static string PSICRO_SET_QUOTA(
        [ExcelArgument(Description = "Quota (Metri o Piedi) / Altitude (Meters or Feet)")] double quota,
        [ExcelArgument(Description = "Unità: SI (metri/kPa) o IP (piedi/psi)")] object unit)
    {
        try
        {
            // 1. Identificazione del sistema di misura
            bool isIP = false;
            if (unit != null && !(unit is ExcelDna.Integration.ExcelMissing) && !(unit is ExcelDna.Integration.ExcelEmpty))
            {
                if (unit.ToString().Trim().ToUpper() == "IP") isIP = true;
            }
            // 2. Conversione INPUT per la DLL (La DLL vuole sempre metri)
            // Se IP: l'utente ha inserito piedi -> convertiamo in metri
            double quotaMetri = isIP ? quota * 0.3048 : quota;
            // 3. Validazione (basata sui metri per coerenza con i limiti fisici)
            if (quotaMetri < -430 || quotaMetri > 11000)
            {
                return isIP
                    ? $"Error: Altitude out of range ({quota} ft)"
                    : $"Errore: Quota fuori range ({quota} m)";
            }

            // 4. Chiamata alla DLL (Input: metri -> Output: sempre kPa)
            double pKpa = Excel_set_quota(quotaMetri);

            // 5. Conversione OUTPUT per visualizzazione utente
            if (isIP)
            {
                // Convertiamo kPa in psi per il feedback testuale
                double pPsi = pKpa * 0.145038;
                return $"Altitude OK: {quota} ft (P: {pPsi:F3} psi / {pKpa:F3} kPa)";
            }
            else
            {
                // Feedback standard in metri e kPa
                return $"Quota OK: {quota} m (P: {pKpa:F2} kPa)";
            }
        }
        catch (Exception ex)
        {
            return "Errore/Error: " + ex.Message;
        }
    }

    [ExcelFunction(Name = "PSICRO.HELP", Description = "Guida rapida alle funzioni / Quick help guide", Category = "Psicrometria")]
    public static string Psicro_Help()
    {
        StringBuilder sb = new StringBuilder();
        sb.AppendLine("GUIDA PSICRO / PSICRO GUIDE:");
        sb.AppendLine("---------------------------------------------------------------------------------");
        sb.AppendLine("\nFUNZIONE PRINCIPALE / MAIN FUNCTION:\n");
        sb.AppendLine("\t  =PSICRO(P1; V1; P2; V2; Target; [Unit])\n");
        sb.AppendLine("Es. entalpia =PSICRO(\"t\"; 26; \"UR\"; 50; \"h\") = 0.010496");
        sb.AppendLine("Ex. entalpy  =PSICRO(\"tdb\";78,8;\"rh\";50;\"w\";\"ip\") = 0.010496");
        sb.AppendLine("\nPARAMETRI / PARAMETERS:\n");
        sb.AppendLine("- Target:\tProprietà da calcolare / Property to calculate");
        sb.AppendLine("\t(t, ur, x, h, vau,...) anche valori multipli separti da ',' o 'tutto'");
        sb.AppendLine("\t(tdb, rh, w, h, v,...) also multiple values separated by ',' or 'all'");
        sb.AppendLine("- P1, P2:\tSimboli input / Symbols of known properties");
        sb.AppendLine("- V1, V2:\tValori o range delle proprietà / Values or ranges of properties");
        sb.AppendLine("- Unit:\t'SI' (Metrico default) o/or 'IP' (Imperial/ASHRAE)");
        sb.AppendLine("\nSIMBOLI VARIABILI / (VARIABLE SYMBOLS):\n\n");
        sb.AppendLine("t / (tdb)\t: Temp. Secca / Dry Bulb");
        sb.AppendLine("ur / (rh)\t: Umidità Relativa / Relative Humidity");
        sb.AppendLine("x / (w)\t: Titolo / Humidity Ratio");
        sb.AppendLine("h / (h)\t: Entalpia / Enthalpy");
        sb.AppendLine("vau / (v)\t: Volume Specifico / Specific Volume");
        sb.AppendLine("tbu / (twb):Temp. Bulbo Umido / Wet Bulb");
        sb.AppendLine("tr / (tdp)\t: Temp. Rugiada / Dew Point\n");
        sb.AppendLine("---------------------------------------------------------------------------------");
        sb.AppendLine("NOTA: PSICRO è vettoriale e accetta range di celle come input.");
        sb.AppendLine("NOTE: PSICRO is vectorial and accepts cell ranges as input.");
        sb.AppendLine("---------------------------------------------------------------------------------");

        string msg = sb.ToString();
        System.Windows.Forms.MessageBox.Show(msg, "Help PsicroAddIn");

        return "Guida visualizzata / Help displayed.";
    }

    [ExcelFunction(Name = "PSICRO.INFO", Description = "Informazioni sulla versione / Version information", Category = "Psicrometria")]
    public static string Psicro_Info()
    {
        string msg = "PsicroAddIn v1.1 (2026)\n" +
                     "----------------------------------\n" +
                     "Motore / Engine: DLL C Integrata / Integrated C DLL\n" +
                     "Logica / Logic: Multi-unit (SI/IP) - ASHRAE Offset Calibrated\n" +
                     "Platform: Excel-DNA\n" +
                     "Equations: ASHRAE Fundamentals Handbook 1997\n" +
                     "License: GPL3\n\n"+
                     "Auth: Ferdinando Battillocchi\n" +
                     "Contact: ferdinando.battillocchi@gmail.com";

        System.Windows.Forms.MessageBox.Show(msg, "Info PsicroAddIn");
        return "v1.1 - F. Battillocchi";
    }
    [ExcelFunction(Name = "PSICRO.UNITS.INFO", Description = "Dettaglio unità di misura SI/IP / SI/IP units detail", Category = "Psicrometria")]
    public static string Psicro_Units_Info()
    {
        StringBuilder sb = new StringBuilder();
        sb.AppendLine("UNITÀ DI MISURA / MEASUREMENT UNITS:");
        sb.AppendLine("---------------------------------------------------------------------------------");
        sb.AppendLine("Var\tDescrizione (IT/EN)\t\tSI\tIP");
        sb.AppendLine("---------------------------------------------------------------------------------");
        sb.AppendLine("t/tdb\tTemp. Secca / Dry Bulb\t°C\t°F");
        sb.AppendLine("ur/rh \tUmidità Rel. / Rel. Humidity\t%\t%");
        sb.AppendLine("x/w\tTitolo / Hum. Ratio\t\tkg/kg\tlb/lb");
        sb.AppendLine("h/h\tEntalpia / Enthalpy\t\tkJ/kg\tBTU/lb");
        sb.AppendLine("vau/v\tVol. Specifico / Spec. Vol.\tm³/kg\tft³/lb");
        sb.AppendLine("tbu/twb\tTemp. B. Umido / Wet Bulb\t°C\t°F");
        sb.AppendLine("tr/tdp \tTemp. Rugiada / Dew Point\t°C\t°F");
        sb.AppendLine("---------------------------------------------------------------------------------");
        sb.AppendLine("\nNota / Note:\n");
        sb.AppendLine("- IP Enthalpy ref: 0 BTU/lb @ 0°F (ASHRAE)");
        sb.AppendLine("- SI Enthalpy ref: 0 kJ/kg @ 0°C");

        string msg = sb.ToString();
        System.Windows.Forms.MessageBox.Show(msg, "Units Info - PsicroAddIn");

        return "Dettaglio unità visualizzato / Units detail displayed.";
    }
       #endregion

}


[ComVisible(true)]
public class PsicroRibbon : ExcelRibbon
{
    // Questo metodo definisce l'aspetto del Ribbon usando il linguaggio XML
    public override string GetCustomUI(string RibbonID)
    {
        return @"
      <customUI xmlns='http://schemas.microsoft.com/office/2009/07/customui'>
        <ribbon>
          <tabs>
            <tab id='PsicroTab' label='PSICROMETRIA'>
              <group id='grpSupport' label='Supporto e Guida'>
                <button id='btnHelp' label='Guida Rapida' imageMso='Help' size='large' onAction='OnHelpClick' />
                <button id='btnUnits' label='Unità di Misura' imageMso='VariableInsert' size='large' onAction='OnUnitsClick' />
                <button id='btnInfo' label='Informazioni' imageMso='Info' size='normal' onAction='OnInfoClick' />
              </group>
            </tab>
          </tabs>
        </ribbon>
      </customUI>";
    }

    // Azioni collegate ai pulsanti
    public void OnHelpClick(IRibbonControl control)
    {
        // Richiama la funzione Help esistente
        PsicroAddIn.Psicro_Help();
    }

    public void OnUnitsClick(IRibbonControl control)
    {
        // Richiama la funzione Units esistente
        PsicroAddIn.Psicro_Units_Info();
    }

    public void OnInfoClick(IRibbonControl control)
    {
        // Richiama la funzione Info esistente
        PsicroAddIn.Psicro_Info();
    }

}

