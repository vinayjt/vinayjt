#include <iostream>
#include <fstream>

class  prepareSubString {
    private:  
    string subscribeTo; 
    public:
    string prepareSubscription(bool flag = true) {
        if(flag) {

            //TCS  INFOSYS TataMOTORS  NTPC  
            string scrip_index = "nse_cm|11536&nse_cm|1594&nse_cm|3456&nse_cm|11630";
            scrip_index += "&nse_cm|16675"; // Bajaj finserver
            scrip_index += "&nse_cm|526"; // Bharat petroleum 
            scrip_index += "&nse_cm|1333"; // HDFC bank  
            scrip_index += "&nse_cm|10604"; // Bharti airtel 
            scrip_index += "&nse_cm|4963"; // ICICI BANK 
            scrip_index += "&nse_cm|16669"; // Bajaj Auto 
            scrip_index += "&nse_cm|2031"; // M&M 
            scrip_index += "&nse_cm|7229"; // HCLTECH 12
            
            scrip_index += "&nse_cm|5900"; // AXISBANK 13

            scrip_index += "&nse_cm|2885"; // RELIANCE 14

            scrip_index += "&nse_cm|1232"; // GRASIM 15

            scrip_index += "&nse_cm|13538"; // TECH Mahindra 16

            scrip_index += "&nse_cm|1348"; // HERO MOTOR CO 17

            scrip_index += "&nse_cm|881"; // DRREDY 18

            scrip_index += "&nse_cm|21808"; // SBILIFE 19

            scrip_index += "&nse_cm|3351"; // SUNPHARMA 20

            scrip_index += "&nse_cm|3045"; // SBIN 21

            scrip_index += "&nse_cm|25"; // ADANI ENT 22

            scrip_index += "&nse_cm|910"; // EICHER MOT 23

            scrip_index += "&nse_cm|17963"; // NESTLE 24

            scrip_index += "&nse_cm|3787"; // WIPRO 25

            scrip_index += "&nse_cm|1708"; // MARUTI 26
            scrip_index += "&nse_cm|11532"; // ULTRAEMCO 27
            scrip_index += "&nse_cm|20374"; // COAL INDIA 28
            scrip_index += "&nse_cm|17818"; // LT MINDTREE 29
            scrip_index += "&nse_cm|11723"; // JSWS STEEL 30
            scrip_index += "&nse_cm|3499"; // TATA STEEL 31
            scrip_index += "&nse_cm|11287"; // UPL 32
            scrip_index += "&nse_cm|694"; //  CIPLA 33
            scrip_index += "&nse_cm|157"; //  APOLLO HOSP 34
            scrip_index += "&nse_cm|1394"; // HIND UNILIVER 35
            scrip_index += "&nse_cm|236"; // ASIAN PAINT 36
            scrip_index += "&nse_cm|1660"; // ITC 37
            scrip_index += "&nse_cm|1922"; // KOTAK BANK 38
            scrip_index += "&nse_cm|11483"; // LT 39
            scrip_index += "&nse_cm|317"; // BAJ FIN 40
            scrip_index += "&nse_cm|547"; // BRITANNIA 41
            scrip_index += "&nse_cm|3432"; // TATA CONSUM 42
            scrip_index += "&nse_cm|5258"; // INDUS IND BANK 43
            scrip_index += "&nse_cm|14977"; // POWER GRID 44
            scrip_index += "&nse_cm|2475"; // ONGC 45
            scrip_index += "&nse_cm|3506"; // TITAN 46
            scrip_index += "&nse_cm|15083"; // ADANI PORTS 47
            scrip_index += "&nse_cm|1363"; // HINDALCO 48
            scrip_index += "&nse_cm|467"; //  HDFCLIFE 49
            scrip_index += "&nse_cm|10940"; //  DIVISLAB 50
             return scrip_index;
        }

    return "" ;
    }

};