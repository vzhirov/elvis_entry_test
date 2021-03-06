#include "parser.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

struct EnumValue
{
    int         mValue;
    const char* mDescription;

    EnumValue(int value, const char* description)
    {
        mValue = value;
        mDescription = description;
    }
};

class EnumDict
{
    typedef std::list<EnumValue> EnumValues;
    std::string mName;
    EnumValues mValues;

public:

    EnumDict(const char* name)
    {
        mName = name;
    }

    EnumDict& operator << ( const EnumValue& value )
    {
        mValues.push_back(value);
        return *this;
    }

    std::string getDescription(int value) const
    {
        std::ostringstream oss;
        oss << std::setw(16) << std::left << mName << ": ";
        EnumValues::const_iterator it;
        for(it=mValues.begin();it!=mValues.end();it++)
            if(it->mValue==value){
                oss << it->mDescription;
                return oss.str(); }
        oss << "unknown("<<value<<")";
        return oss.str();
    }

};

unsigned char  EDID_SIGNATURE[8] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };

EnumDict EDID_DIGITAL_BIT_DEPTHS = EnumDict("Bit depth")
                                        << EnumValue( 0,"undefined" )
                                        << EnumValue(1,"6")
                                        << EnumValue(2,"8")
                                        << EnumValue(3,"10")
                                        << EnumValue(4,"12")
                                        << EnumValue(5,"14")
                                        << EnumValue(6,"16 bits per color")
                                        << EnumValue(7,"reserved") ;

EnumDict EDID_DIGITAL_VIDEO = EnumDict("Video interface")
                                        << EnumValue( 0,"undefined" )
                                        << EnumValue( 2,"HDMIa" )
                                        << EnumValue( 3,"HDMIb" )
                                        << EnumValue( 4,"MDDI" )
                                        << EnumValue( 5,"DisplayPort" ) ;


EnumDict EDID_DIGITAL_VIDEO_LEVELS = EnumDict("Video white and sync levels")
                                        << EnumValue( 0,"+0.7/−0.3 V" )
                                        << EnumValue( 1,"+0.714/−0.286 V" )
                                        << EnumValue( 2,"+1.0/−0.4 V" )
                                        << EnumValue( 3,"+0.7/0 V" );

EnumDict EDID_DIGIAL_TYPES = EnumDict("Display type")
                                        << EnumValue( 0,"RGB 4:4:4" )
                                        << EnumValue( 1,"RGB 4:4:4 + YCrCb 4:4:4" )
                                        << EnumValue( 2,"RGB 4:4:4 + YCrCb 4:2:2" )
                                        << EnumValue( 3,"RGB 4:4:4 + YCrCb 4:4:4 + YCrCb 4:2:2" );

EnumDict EDID_ANALOG_TYPES = EnumDict("Display type")
                                        << EnumValue( 0,"monochrome or grayscale" )
                                        << EnumValue( 1,"RGB color" )
                                        << EnumValue( 2,"non-RGB color" )
                                        << EnumValue( 3,"undefined" );


EnumDict EDID_ESTABLISHED_TIMING = EnumDict("Established timing")
                                        << EnumValue( 0x07,"720×400 @ 70 Hz (VGA)" )
                                        << EnumValue( 0x06,"720×400 @ 88 Hz (XGA)" )
                                        << EnumValue( 0x05,"640×480 @ 60 Hz (VGA) " )
                                        << EnumValue( 0x04,"640×480 @ 67 Hz (Apple Macintosh II)" )
                                        << EnumValue( 0x03,"640×480 @ 72 Hz" )
                                        << EnumValue( 0x02,"640×480 @ 75 Hz" )
                                        << EnumValue( 0x01,"800×600 @ 56 Hz" )
                                        << EnumValue( 0x00,"800×600 @ 60 Hz" )

                                        << EnumValue( 0x17,"800×600 @ 72 Hz" )
                                        << EnumValue( 0x16,"800×600 @ 75 Hz" )
                                        << EnumValue( 0x15,"832×624 @ 75 Hz (Apple Macintosh II)" )
                                        << EnumValue( 0x14,"1024×768 @ 87 Hz, interlaced (1024×768i)" )
                                        << EnumValue( 0x13,"1024×768 @ 60 Hz" )
                                        << EnumValue( 0x12,"1024×768 @ 70 Hz" )
                                        << EnumValue( 0x11,"1024×768 @ 75 Hz" )
                                        << EnumValue( 0x10,"1280×1024 @ 75 Hz" )

                                        << EnumValue( 0x27,"1152x870 @ 75 Hz (Apple Macintosh II)" );


std::string manufacturerChar(int id,int shift)
{
    char result[2] = { 'A', 0 };
    result[0] += ((id >> shift) & 0x1F)-1;
    return result;
}

std::string hex(int value)
{
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setfill('0') << std::uppercase << value;
    return oss.str();
}

bool bit2bool(int value, int shift)
{
    return value & (1<<shift);
}

void parseEdid(Parser& parser, std::ostream& out)
{
    using namespace std;
    int checkSum;
    int manufacturerId;
    int productCode;
    int serialNumber;
    int week;
    int year;
    int version[2];
    int displayBits;
    bool digitalInput;
    bool blankToBlackSetup;
    bool separateSync;
    bool compositeSync;
    bool syncOnGreen;
    bool vSyncPulse;
    bool dmpsStandby;
    bool dmpsSuspend;
    bool dmpsActiveOff;
    bool sRgbColor;
    bool prefferedTiming;
    bool continiousTiming;
    int hScreenSize, vScreenSize;
    int gamma;
    int dpmsValue;
    int rX,rY, gX, gY, bX, bY, wX, wY;
    int establishedTimings[3];

    bool hasEstablishedTimings = false;

    checkSum = parser.byteSum(128);
    parser.parseSignature(EDID_SIGNATURE,sizeof(EDID_SIGNATURE));
    if(checkSum!=0)
        parser.setError("Invalid checksum");

    parser.parseI16LE(&manufacturerId);
    parser.parseI16LE(&productCode);
    parser.parseI32LE(&serialNumber);
    parser.parseI8(&week);
    parser.parseI8(&year); year+=1990;
    parser.parseI8(&version[0]);
    parser.parseI8(&version[1]);
    parser.parseI8(&displayBits);
    digitalInput = bit2bool(displayBits,7);
    if(!digitalInput){
        blankToBlackSetup = bit2bool(displayBits,4);
        separateSync      = bit2bool(displayBits,3);
        compositeSync     = bit2bool(displayBits,2);
        syncOnGreen       = bit2bool(displayBits,1);
        vSyncPulse        = bit2bool(displayBits,0); }
    parser.parseI8(&hScreenSize);
    parser.parseI8(&vScreenSize);
    parser.parseI8(&gamma);
    parser.parseI8(&dpmsValue);
        dmpsStandby      = bit2bool(dpmsValue,7);
        dmpsSuspend      = bit2bool(dpmsValue,6);
        dmpsActiveOff    = bit2bool(dpmsValue,5);
        sRgbColor        = bit2bool(dpmsValue,2);
        prefferedTiming  = bit2bool(dpmsValue,1);
        continiousTiming = bit2bool(dpmsValue,0);
    rX = (parser[2]<<2)+((parser[0]>>6) & 3);
    rY = (parser[3]<<2)+((parser[0]>>4) & 3);
    gX = (parser[4]<<2)+((parser[0]>>2) & 3);
    gY = (parser[5]<<2)+((parser[0]>>0) & 3);
    bX = (parser[6]<<2)+((parser[1]>>6) & 3);
    bY = (parser[7]<<2)+((parser[0]>>4) & 3);
    wX = (parser[8]<<2)+((parser[0]>>2) & 3);
    wY = (parser[9]<<2)+((parser[0]>>0) & 3);
    parser.skip(10);
    parser.parseI8(&establishedTimings[0]);
    parser.parseI8(&establishedTimings[1]);
    parser.parseI8(&establishedTimings[2]);
    // parser.setError("TODO");

    out << "Manufacturer    : " << manufacturerChar(manufacturerId,10)
                             << manufacturerChar(manufacturerId,5)
                             << manufacturerChar(manufacturerId,0)
                             << " (hex: " << hex(manufacturerId) << ")"
                             << endl;
    out << "Product code    : " << productCode << endl;
    out << "Serial number   : " << hex(serialNumber) << endl;
    out << "Production date : " << year;
    if(week!=0xFF)
        out << " (week = " << week << ")";
    out << endl;
    out << "Version         : " << version[0] << "." << version[1] << endl;
    out << "Video input parameters: ";
    if(digitalInput){
        out << "digital" << endl
            << EDID_DIGITAL_BIT_DEPTHS.getDescription((displayBits>>4)&0x7) << endl
            << EDID_DIGITAL_VIDEO.getDescription((displayBits>>0)&0x7) << endl
            << EDID_DIGIAL_TYPES.getDescription((dpmsValue>>3)&3) << endl;
    }else{
        out << "analog" << endl
            << "                :"
            << EDID_DIGITAL_VIDEO_LEVELS.getDescription((displayBits>>0)&0x3) << endl
            << EDID_ANALOG_TYPES.getDescription((dpmsValue>>3)&3) << endl;
        if(blankToBlackSetup)
            out << "                  "
                << "Blank-to-black setup (pedestal) expected" << endl;
        if(separateSync)
            out << "                  "
                << "Separate sync supported" << endl;
        if(compositeSync)
            out << "                  "
                << "Composite sync (on HSync) supported" << endl;
        if(syncOnGreen)
            out << "                  "
                << "Sync on green supported" << endl;
        if(vSyncPulse)
            out << "                  "
                << "VSync pulse must be serrated when composite or sync-on-green is used" << endl; }
    out << "Screen size     : " << hScreenSize << " x " << vScreenSize << " sm" << endl;
    out << "Display gamma   : " << std::fixed << std::setprecision(2) << gamma*0.01+1 << endl;
    out << "Chromaticity    : " << "R("<<rX<<"/"<<rY<<") " << "G("<<gX<<"/"<<gY<<") "
                                << "B("<<bX<<"/"<<bY<<") " << "W("<<wX<<"/"<<wY<<") "
                                << endl;
    for(int iByte=0;iByte<3;iByte++)
        for(int iBit=0;iBit<8;iBit++)
            if(establishedTimings[iByte] & (1<<iBit)){
                if(!hasEstablishedTimings)
                    out << "Established timing : ";
                else
                    out << "                     ";
                out << EDID_ESTABLISHED_TIMING.getDescription(iByte*16+iBit) << endl; }
//                if(!hasEstablishedTimings)
//                    out << "Established timing : ";
//                else
//                    out << "                     ";
//                out << EDID_ESTABLISHED_TIMING.getDescription(iByte*16+iBit) << endl;
// }
//        

    out << "DPMS features   :";
    if(dmpsStandby)
        out <<" standby";
    if(dmpsSuspend)
        out <<" suspend";
    if(dmpsActiveOff)
        out <<" activeOff";
    out << endl;
    if(sRgbColor || prefferedTiming || continiousTiming){
        out << "Other features  : " << endl;
        if(sRgbColor)
            out << "                  "
                << "Standard sRGB colour space" <<endl;
        if(prefferedTiming)
            out << "                  "
                << "Preferred timing mode" <<endl;
        if(continiousTiming)
            out << "                  "
                << "Continuous timings" <<endl; }

    out << "Checksum        : " << checkSum << " "
                                << (checkSum==0 ? "(valid)" : "(invalid)")
                                << endl;
    out << "Done." << endl;
}

void processFile(const std::string& path)
{
    using namespace std;
    fstream file(path, ios_base::in | ios_base::binary);
    if(file.fail()){
        cerr << "[ERROR] Can't open edid file(" << path << ")" << endl;
        return; }
    cout << "# ========================================================" << endl;
    cout << "# == " << path << endl;
    cout << "# ========================================================" << endl;
    vector<unsigned char> buffer;
    for(int nBytes = 0;file.good();){
        buffer.insert(buffer.end(),1024,0);
        file.read((char*)&buffer.at(nBytes),buffer.size()-nBytes);
        nBytes += file.gcount();
        buffer.erase(buffer.begin()+nBytes,buffer.end()); }
    Parser parser(buffer.data(),buffer.size());
    std::ostringstream output;
    parseEdid(parser,output);
    if(parser.isError())
        cerr << "[ERROR] file format error:" << endl
             << parser.errorMessage() << endl;
    else
        cout << output.str();
}

int main(int argc,char *argv[])
{
    using namespace std;
    if(argc==1){
        cerr << "Warning!!! Require at least one argument" << endl;
        cerr << "USAGE: elvis_vzhirov_bin edid-files ..." << endl;
        return -1; }
    for(int i=1; i<argc;i++)
        processFile(argv[i]);
    return 0;
}

// vim: et ts=4

