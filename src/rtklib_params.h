#ifndef RTKLIB_PARAMS_H
#define RTKLIB_PARAMS_H

#include <string>
#include <vector>

/* Class that holds vectors of parameter names. */

class RtklibParams{
    public:     
    std::vector<std::string> rtklib_string_params;
    std::vector<std::string> rtklib_int_params;
    std::vector<std::string> rtklib_double_params;

    
    RtklibParams()
    {
        rtklib_string_params.push_back("pos1_posmode");  //kinematic  # (0:single,1:dgps,2:kinematic,3:static,4:movingbase,5:fixed,6:ppp_kine,7:ppp_static)
        rtklib_string_params.push_back("pos1_frequency");  //l1         # (1:l1,2:l1+l2,3:l1+l2+l5,4:l1+l2+l5+l6,5:l1+l2+l5+l6+l7)
        rtklib_string_params.push_back("pos1_soltype");  //forward    # (0:forward,1:backward,2:combined)
        rtklib_string_params.push_back("pos1_dynamics");  //off        # (0:off,1:on)
        rtklib_string_params.push_back("pos1_tidecorr");  //off        # (0:off,1:on)
        rtklib_string_params.push_back("pos1_ionoopt");  //brdc       # 0:off,1:brdc,2:sbas,3:dual_freq,4:est_stec,5:ionex_tec,6:qzs_brdc,7:qzs_lex,8:vtec_sf,9:vtec_ef,10:gtec)
        rtklib_string_params.push_back("pos1_tropopt");  //saas       # (0:off,1:saas,2:sbas,3:est_ztd,4:est_ztdgrad)
        rtklib_string_params.push_back("pos1_sateph");  //brdc       # (0:brdc,1:precise,2:brdc+sbas,3:brdc+ssrapc,4:brdc+ssrcom)
        rtklib_string_params.push_back("pos1_exclsats");  //''            # (prn ...)
        rtklib_string_params.push_back("pos2_armode");  //continuous # (0:off,1:continuous,2:instantaneous,3:fix_and_hold)
        rtklib_string_params.push_back("pos2_gloarmode");  //autocal    # (0:off,1:on,2:autocal)
        rtklib_string_params.push_back("ant1_postype");  //llh        # (0:llh,1:xyz,2:single,3:posfile,4:rinexhead,5:rtcm)
        rtklib_string_params.push_back("ant1_anttype");  //'' # antenna types specified in rtklib documentation
        rtklib_string_params.push_back("ant2_postype");  //llh        # (0:llh,1:xyz,2:single,3:posfile,4:rinexhead,5:rtcm)
        rtklib_string_params.push_back("ant2_anttype");  //''
        rtklib_string_params.push_back("misc_timeinterp");  //off        # (0:off,1:on)
        rtklib_string_params.push_back("file_satantfile");  //''
        rtklib_string_params.push_back("file_rcvantfile");  //''
        rtklib_string_params.push_back("file_staposfile");  //''
        rtklib_string_params.push_back("file_geoidfile");  //''
        rtklib_string_params.push_back("file_ionofile");  //''
        rtklib_string_params.push_back("file_dcbfile");  //''
        rtklib_string_params.push_back("file_tempdir");  //tmp
        rtklib_string_params.push_back("file_geexefile");  //''
        rtklib_string_params.push_back("file_solstatfile");  //''
        rtklib_string_params.push_back("file_tracefile");  //''
        rtklib_string_params.push_back("inpstr1_type");  //serial     # ROVER (0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,7:ntripcli,8:ftp,9:http)
        rtklib_string_params.push_back("inpstr1_path");  //'ttyACM0:57600:8:n:1:off' # @ADAPT");  //CHANGE ROVER MEAS SOURCE
        rtklib_string_params.push_back("inpstr1_format");  //ubx       # (0:rtcm2,1:rtcm3,2:oem4,3:oem3,4:ubx,5:ss2,6:hemis,7:skytraq,8:gw10,9:javad,15:sp3)
        rtklib_string_params.push_back("inpstr2_type");  //tcpcli     # BASE (0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,7:ntripcli,8:ftp,9:http)
        rtklib_string_params.push_back("inpstr2_path");  //':@localhost:8600/:' # @ADAPT");  //CHANGE BASE MEAS SOURCE
        rtklib_string_params.push_back("inpstr2_format");  //ubx        #(0:rtcm2,1:rtcm3,2:oem4,3:oem3,4:ubx,5:ss2,6:hemis,7:skytraq,8:gw10,9:javad,15:sp3)
        rtklib_string_params.push_back("inpstr2_nmeareq");  //off        # (0:off,1:latlon,2:single)
        rtklib_string_params.push_back("inpstr3_type");  //off        # (0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,7:ntripcli,8:ftp,9:http)
        rtklib_string_params.push_back("inpstr3_path");  //''
        rtklib_string_params.push_back("inpstr3_format");  //rtcm2      # (0:rtcm2,1:rtcm3,2:oem4,3:oem3,4:ubx,5:ss2,6:hemis,7:skytraq,8:gw10,9:javad,15:sp3)
        rtklib_string_params.push_back("outstr1_type");  //tcpsvr        # (0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripsvr)
        rtklib_string_params.push_back("outstr1_path");  //':@:8601/:'  # @ADAPT");  //SOLUTION IS OUTPUT HERE AND via ros @ topic "/baseline"
        rtklib_string_params.push_back("outstr1_format");  //enu        # (0:llh,1:xyz,2:enu,3:nmea)
        rtklib_string_params.push_back("outstr2_type");  //off        # (0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripsvr)
        rtklib_string_params.push_back("outstr2_path");  //''
        rtklib_string_params.push_back("outstr2_format");  //enu        # (0:llh,1:xyz,2:enu,3:nmea)
        rtklib_string_params.push_back("out_solformat");  //enu        # (0:llh,1:xyz,2:enu,3:nmea)
        rtklib_string_params.push_back("out_outhead");  //on         # (0:off,1:on)
        rtklib_string_params.push_back("out_outopt");  //off        # (0:off,1:on)
        rtklib_string_params.push_back("out_timesys");  //gpst       # (0:gpst,1:utc,2:jst)
        rtklib_string_params.push_back("out_timeform");  //hms        # (0:tow,1:hms)
        rtklib_string_params.push_back("out_degform");  //deg        # (0:deg,1:dms)
        rtklib_string_params.push_back("out_fieldsep");  //''
        rtklib_string_params.push_back("out_height");  //ellipsoidal # (0:ellipsoidal,1:geodetic)
        rtklib_string_params.push_back("out_geoid");  //internal   # (0:internal,1:egm96,2:egm08_2.5,3:egm08_1,4:gsi2000)
        rtklib_string_params.push_back("out_solstatic");  //all        # (0:all,1:single)
        rtklib_string_params.push_back("out_outstat");  //off        # (0:off,1:state,2:residual)
        rtklib_string_params.push_back("logstr1_type");  //file       # (0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripsvr)
        rtklib_string_params.push_back("logstr1_path");  //'$(find rtklib)/logs/%Y%m%d_%h%M_rover_rtk_server'
        rtklib_string_params.push_back("logstr2_type");  //file       # (0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripsvr)
        rtklib_string_params.push_back("logstr2_path");  //'$(find rtklib)/logs/%Y%m%d_%h%M_base_rtk_server'
        rtklib_string_params.push_back("logstr3_type");  //off        # (0:off,1:serial,2:file,3:tcpsvr,4:tcpcli,6:ntripsvr)
        rtklib_string_params.push_back("logstr3_path");  //''
        rtklib_string_params.push_back("misc_navmsgsel");  //all        # (0:all,1:rover,2:base,3:corr)
        rtklib_string_params.push_back("misc_proxyaddr"); // ''


        rtklib_int_params.push_back("pos1_navsys"); //5          # (1:gps+2:sbas+4:glo+8:gal+16:qzs+32:comp)
        rtklib_int_params.push_back("pos2_arlockcnt"); //0
        rtklib_int_params.push_back("pos2_arminfix"); //10
        rtklib_int_params.push_back("pos2_aroutcnt"); //5
        rtklib_int_params.push_back("pos2_niter"); //1
        rtklib_int_params.push_back("misc_sbasatsel"); //0          # (0:all)
        rtklib_int_params.push_back("out_timendec"); //3
        rtklib_int_params.push_back("misc_svrcycle"); //50         # (ms)
        rtklib_int_params.push_back("misc_timeout"); //10000      # (ms)
        rtklib_int_params.push_back("misc_reconnect"); //10000      # (ms)
        rtklib_int_params.push_back("misc_nmeacycle"); //5000       # (ms)
        rtklib_int_params.push_back("misc_buffsize"); //32768      # (bytes)
        rtklib_int_params.push_back("misc_fswapmargin"); // 30         # (s)


        rtklib_double_params.push_back("pos2_slipthres"); //0.05       # (m)
        rtklib_double_params.push_back("pos2_rejionno"); //30         # (m)
        rtklib_double_params.push_back("pos2_rejgdop"); //30
        rtklib_double_params.push_back("pos2_baselen"); //0          # (m)
        rtklib_double_params.push_back("pos2_basesig"); //0          # (m)
        rtklib_double_params.push_back("ant1_pos1"); //0  # (deg|m) @ADAPT:STATION 1 POSITION
        rtklib_double_params.push_back("ant1_pos2"); //0  # (deg|m) @ADAPT:STATION 1 POSITION
        rtklib_double_params.push_back("ant1_pos3"); //0  # (m|m) @ADAPT:STATION 1 POSITION
        rtklib_double_params.push_back("ant1_antdele"); //0          # (m)
        rtklib_double_params.push_back("ant1_antdeln"); //0          # (m)
        rtklib_double_params.push_back("ant1_antdelu"); //0          # (m)
        rtklib_double_params.push_back("ant2_pos1"); //0  # (deg|m) # @ADAPT:STATION 2 POSITION
        rtklib_double_params.push_back("ant2_pos2"); //0  # (deg|m) # @ADAPT:STATION 2 POSITION
        rtklib_double_params.push_back("ant2_pos3"); //0  # (m|m)	# @ADAPT:STATION 2 POSITION
        rtklib_double_params.push_back("ant2_antdele"); //0          # (m)
        rtklib_double_params.push_back("ant2_antdeln"); //0          # (m)
        rtklib_double_params.push_back("ant2_antdelu"); //0          # (m)
        rtklib_double_params.push_back("inpstr2_nmealat"); //0          # (deg)
        rtklib_double_params.push_back("inpstr2_nmealon"); //0          # (deg)
        rtklib_double_params.push_back("out_nmeaintv1"); //0          # (s)
        rtklib_double_params.push_back("out_nmeaintv2"); //0          # (s)
        rtklib_double_params.push_back("stats_eratio1"); //100
        rtklib_double_params.push_back("stats_eratio2"); //100
        rtklib_double_params.push_back("stats_errphase"); //0.003      # (m)
        rtklib_double_params.push_back("stats_errphaseel"); //0.003      # (m)
        rtklib_double_params.push_back("stats_errphasebl"); //0          # (m/10km)
        rtklib_double_params.push_back("stats_errdoppler"); //1          # (Hz)
        rtklib_double_params.push_back("stats_stdbias"); //30         # (m)
        rtklib_double_params.push_back("stats_stdiono"); //0.03       # (m)
        rtklib_double_params.push_back("stats_stdtrop"); //0.3        # (m)
        rtklib_double_params.push_back("stats_prnaccelh"); //10         # (m/s^2)
        rtklib_double_params.push_back("stats_prnaccelv"); //10         # (m/s^2)
        rtklib_double_params.push_back("stats_prnbias"); //0.0001     # (m)
        rtklib_double_params.push_back("stats_prniono"); //0.001      # (m)
        rtklib_double_params.push_back("stats_prntrop"); //0.0001     # (m)
        rtklib_double_params.push_back("stats_clkstab"); //5e-12      # (s/s)
        rtklib_double_params.push_back("pos2_maxage"); //30         # (s)
        rtklib_double_params.push_back("pos2_elmaskhold"); //0          # (deg)
        rtklib_double_params.push_back("pos2_arelmask"); //0          # (deg)
        rtklib_double_params.push_back("pos2_arthres"); //3
        rtklib_double_params.push_back("pos1_elmask"); //15         # (deg)
        rtklib_double_params.push_back("pos1_snrmask"); //0          # (dBHz)
    }

};

#endif // RTKLIB_PARAMS_H
