// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QByteArray>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <map>
#include <vector>
#include <array>
#include <string>
#include "configwidget.h"
#include "item.h"
#include "main.h"
#include "standardaction.h"
#include "standarditem.h"
#include "query.h"
#include "xdgiconlookup.h"
using std::shared_ptr;
using std::vector;
using namespace Core;


namespace {

// http://data.iana.org/TLD/tlds-alpha-by-domain.txt
// Version 2017032500, Last Updated Sat Mar 25 07:07:02 2017 UTC
std::array<std::string, 1530> validTlds = { {"AAA", "AARP", "ABARTH", "ABB",
"ABBOTT", "ABBVIE", "ABC", "ABLE", "ABOGADO", "ABUDHABI", "AC", "ACADEMY",
"ACCENTURE", "ACCOUNTANT", "ACCOUNTANTS", "ACO", "ACTIVE", "ACTOR", "AD",
"ADAC", "ADS", "ADULT", "AE", "AEG", "AERO", "AETNA", "AF", "AFAMILYCOMPANY",
"AFL", "AFRICA", "AG", "AGAKHAN", "AGENCY", "AI", "AIG", "AIGO", "AIRBUS",
"AIRFORCE", "AIRTEL", "AKDN", "AL", "ALFAROMEO", "ALIBABA", "ALIPAY",
"ALLFINANZ", "ALLSTATE", "ALLY", "ALSACE", "ALSTOM", "AM", "AMERICANEXPRESS",
"AMERICANFAMILY", "AMEX", "AMFAM", "AMICA", "AMSTERDAM", "ANALYTICS",
"ANDROID", "ANQUAN", "ANZ", "AO", "AOL", "APARTMENTS", "APP", "APPLE", "AQ",
"AQUARELLE", "AR", "ARAMCO", "ARCHI", "ARMY", "ARPA", "ART", "ARTE", "AS",
"ASDA", "ASIA", "ASSOCIATES", "AT", "ATHLETA", "ATTORNEY", "AU", "AUCTION",
"AUDI", "AUDIBLE", "AUDIO", "AUSPOST", "AUTHOR", "AUTO", "AUTOS", "AVIANCA",
"AW", "AWS", "AX", "AXA", "AZ", "AZURE", "BA", "BABY", "BAIDU", "BANAMEX",
"BANANAREPUBLIC", "BAND", "BANK", "BAR", "BARCELONA", "BARCLAYCARD",
"BARCLAYS", "BAREFOOT", "BARGAINS", "BASEBALL", "BASKETBALL", "BAUHAUS",
"BAYERN", "BB", "BBC", "BBT", "BBVA", "BCG", "BCN", "BD", "BE", "BEATS",
"BEAUTY", "BEER", "BENTLEY", "BERLIN", "BEST", "BESTBUY", "BET", "BF", "BG",
"BH", "BHARTI", "BI", "BIBLE", "BID", "BIKE", "BING", "BINGO", "BIO", "BIZ",
"BJ", "BLACK", "BLACKFRIDAY", "BLANCO", "BLOCKBUSTER", "BLOG", "BLOOMBERG",
"BLUE", "BM", "BMS", "BMW", "BN", "BNL", "BNPPARIBAS", "BO", "BOATS",
"BOEHRINGER", "BOFA", "BOM", "BOND", "BOO", "BOOK", "BOOKING", "BOOTS",
"BOSCH", "BOSTIK", "BOSTON", "BOT", "BOUTIQUE", "BOX", "BR", "BRADESCO",
"BRIDGESTONE", "BROADWAY", "BROKER", "BROTHER", "BRUSSELS", "BS", "BT",
"BUDAPEST", "BUGATTI", "BUILD", "BUILDERS", "BUSINESS", "BUY", "BUZZ", "BV",
"BW", "BY", "BZ", "BZH", "CA", "CAB", "CAFE", "CAL", "CALL", "CALVINKLEIN",
"CAM", "CAMERA", "CAMP", "CANCERRESEARCH", "CANON", "CAPETOWN", "CAPITAL",
"CAPITALONE", "CAR", "CARAVAN", "CARDS", "CARE", "CAREER", "CAREERS", "CARS",
"CARTIER", "CASA", "CASE", "CASEIH", "CASH", "CASINO", "CAT", "CATERING",
"CATHOLIC", "CBA", "CBN", "CBRE", "CBS", "CC", "CD", "CEB", "CENTER", "CEO",
"CERN", "CF", "CFA", "CFD", "CG", "CH", "CHANEL", "CHANNEL", "CHASE", "CHAT",
"CHEAP", "CHINTAI", "CHLOE", "CHRISTMAS", "CHROME", "CHRYSLER", "CHURCH", "CI",
"CIPRIANI", "CIRCLE", "CISCO", "CITADEL", "CITI", "CITIC", "CITY", "CITYEATS",
"CK", "CL", "CLAIMS", "CLEANING", "CLICK", "CLINIC", "CLINIQUE", "CLOTHING",
"CLOUD", "CLUB", "CLUBMED", "CM", "CN", "CO", "COACH", "CODES", "COFFEE",
"COLLEGE", "COLOGNE", "COM", "COMCAST", "COMMBANK", "COMMUNITY", "COMPANY",
"COMPARE", "COMPUTER", "COMSEC", "CONDOS", "CONSTRUCTION", "CONSULTING",
"CONTACT", "CONTRACTORS", "COOKING", "COOKINGCHANNEL", "COOL", "COOP",
"CORSICA", "COUNTRY", "COUPON", "COUPONS", "COURSES", "CR", "CREDIT",
"CREDITCARD", "CREDITUNION", "CRICKET", "CROWN", "CRS", "CRUISE", "CRUISES",
"CSC", "CU", "CUISINELLA", "CV", "CW", "CX", "CY", "CYMRU", "CYOU", "CZ",
"DABUR", "DAD", "DANCE", "DATA", "DATE", "DATING", "DATSUN", "DAY", "DCLK",
"DDS", "DE", "DEAL", "DEALER", "DEALS", "DEGREE", "DELIVERY", "DELL",
"DELOITTE", "DELTA", "DEMOCRAT", "DENTAL", "DENTIST", "DESI", "DESIGN", "DEV",
"DHL", "DIAMONDS", "DIET", "DIGITAL", "DIRECT", "DIRECTORY", "DISCOUNT",
"DISCOVER", "DISH", "DIY", "DJ", "DK", "DM", "DNP", "DO", "DOCS", "DOCTOR",
"DODGE", "DOG", "DOHA", "DOMAINS", "DOT", "DOWNLOAD", "DRIVE", "DTV", "DUBAI",
"DUCK", "DUNLOP", "DUNS", "DUPONT", "DURBAN", "DVAG", "DVR", "DZ", "EARTH",
"EAT", "EC", "ECO", "EDEKA", "EDU", "EDUCATION", "EE", "EG", "EMAIL", "EMERCK",
"ENERGY", "ENGINEER", "ENGINEERING", "ENTERPRISES", "EPOST", "EPSON",
"EQUIPMENT", "ER", "ERICSSON", "ERNI", "ES", "ESQ", "ESTATE", "ESURANCE", "ET",
"EU", "EUROVISION", "EUS", "EVENTS", "EVERBANK", "EXCHANGE", "EXPERT",
"EXPOSED", "EXPRESS", "EXTRASPACE", "FAGE", "FAIL", "FAIRWINDS", "FAITH",
"FAMILY", "FAN", "FANS", "FARM", "FARMERS", "FASHION", "FAST", "FEDEX",
"FEEDBACK", "FERRARI", "FERRERO", "FI", "FIAT", "FIDELITY", "FIDO", "FILM",
"FINAL", "FINANCE", "FINANCIAL", "FIRE", "FIRESTONE", "FIRMDALE", "FISH",
"FISHING", "FIT", "FITNESS", "FJ", "FK", "FLICKR", "FLIGHTS", "FLIR",
"FLORIST", "FLOWERS", "FLY", "FM", "FO", "FOO", "FOOD", "FOODNETWORK",
"FOOTBALL", "FORD", "FOREX", "FORSALE", "FORUM", "FOUNDATION", "FOX", "FR",
"FREE", "FRESENIUS", "FRL", "FROGANS", "FRONTDOOR", "FRONTIER", "FTR",
"FUJITSU", "FUJIXEROX", "FUN", "FUND", "FURNITURE", "FUTBOL", "FYI", "GA",
"GAL", "GALLERY", "GALLO", "GALLUP", "GAME", "GAMES", "GAP", "GARDEN", "GB",
"GBIZ", "GD", "GDN", "GE", "GEA", "GENT", "GENTING", "GEORGE", "GF", "GG",
"GGEE", "GH", "GI", "GIFT", "GIFTS", "GIVES", "GIVING", "GL", "GLADE", "GLASS",
"GLE", "GLOBAL", "GLOBO", "GM", "GMAIL", "GMBH", "GMO", "GMX", "GN", "GODADDY",
"GOLD", "GOLDPOINT", "GOLF", "GOO", "GOODHANDS", "GOODYEAR", "GOOG", "GOOGLE",
"GOP", "GOT", "GOV", "GP", "GQ", "GR", "GRAINGER", "GRAPHICS", "GRATIS",
"GREEN", "GRIPE", "GROUP", "GS", "GT", "GU", "GUARDIAN", "GUCCI", "GUGE",
"GUIDE", "GUITARS", "GURU", "GW", "GY", "HAIR", "HAMBURG", "HANGOUT", "HAUS",
"HBO", "HDFC", "HDFCBANK", "HEALTH", "HEALTHCARE", "HELP", "HELSINKI", "HERE",
"HERMES", "HGTV", "HIPHOP", "HISAMITSU", "HITACHI", "HIV", "HK", "HKT", "HM",
"HN", "HOCKEY", "HOLDINGS", "HOLIDAY", "HOMEDEPOT", "HOMEGOODS", "HOMES",
"HOMESENSE", "HONDA", "HONEYWELL", "HORSE", "HOSPITAL", "HOST", "HOSTING",
"HOT", "HOTELES", "HOTMAIL", "HOUSE", "HOW", "HR", "HSBC", "HT", "HTC", "HU",
"HUGHES", "HYATT", "HYUNDAI", "IBM", "ICBC", "ICE", "ICU", "ID", "IE", "IEEE",
"IFM", "IKANO", "IL", "IM", "IMAMAT", "IMDB", "IMMO", "IMMOBILIEN", "IN",
"INDUSTRIES", "INFINITI", "INFO", "ING", "INK", "INSTITUTE", "INSURANCE",
"INSURE", "INT", "INTEL", "INTERNATIONAL", "INTUIT", "INVESTMENTS", "IO",
"IPIRANGA", "IQ", "IR", "IRISH", "IS", "ISELECT", "ISMAILI", "IST", "ISTANBUL",
"IT", "ITAU", "ITV", "IVECO", "IWC", "JAGUAR", "JAVA", "JCB", "JCP", "JE",
"JEEP", "JETZT", "JEWELRY", "JIO", "JLC", "JLL", "JM", "JMP", "JNJ", "JO",
"JOBS", "JOBURG", "JOT", "JOY", "JP", "JPMORGAN", "JPRS", "JUEGOS", "JUNIPER",
"KAUFEN", "KDDI", "KE", "KERRYHOTELS", "KERRYLOGISTICS", "KERRYPROPERTIES",
"KFH", "KG", "KH", "KI", "KIA", "KIM", "KINDER", "KINDLE", "KITCHEN", "KIWI",
"KM", "KN", "KOELN", "KOMATSU", "KOSHER", "KP", "KPMG", "KPN", "KR", "KRD",
"KRED", "KUOKGROUP", "KW", "KY", "KYOTO", "KZ", "LA", "LACAIXA", "LADBROKES",
"LAMBORGHINI", "LAMER", "LANCASTER", "LANCIA", "LANCOME", "LAND", "LANDROVER",
"LANXESS", "LASALLE", "LAT", "LATINO", "LATROBE", "LAW", "LAWYER", "LB", "LC",
"LDS", "LEASE", "LECLERC", "LEFRAK", "LEGAL", "LEGO", "LEXUS", "LGBT", "LI",
"LIAISON", "LIDL", "LIFE", "LIFEINSURANCE", "LIFESTYLE", "LIGHTING", "LIKE",
"LILLY", "LIMITED", "LIMO", "LINCOLN", "LINDE", "LINK", "LIPSY", "LIVE",
"LIVING", "LIXIL", "LK", "LOAN", "LOANS", "LOCKER", "LOCUS", "LOFT", "LOL",
"LONDON", "LOTTE", "LOTTO", "LOVE", "LPL", "LPLFINANCIAL", "LR", "LS", "LT",
"LTD", "LTDA", "LU", "LUNDBECK", "LUPIN", "LUXE", "LUXURY", "LV", "LY", "MA",
"MACYS", "MADRID", "MAIF", "MAISON", "MAKEUP", "MAN", "MANAGEMENT", "MANGO",
"MARKET", "MARKETING", "MARKETS", "MARRIOTT", "MARSHALLS", "MASERATI",
"MATTEL", "MBA", "MC", "MCD", "MCDONALDS", "MCKINSEY", "MD", "ME", "MED",
"MEDIA", "MEET", "MELBOURNE", "MEME", "MEMORIAL", "MEN", "MENU", "MEO",
"METLIFE", "MG", "MH", "MIAMI", "MICROSOFT", "MIL", "MINI", "MINT", "MIT",
"MITSUBISHI", "MK", "ML", "MLB", "MLS", "MM", "MMA", "MN", "MO", "MOBI",
"MOBILE", "MOBILY", "MODA", "MOE", "MOI", "MOM", "MONASH", "MONEY", "MONSTER",
"MONTBLANC", "MOPAR", "MORMON", "MORTGAGE", "MOSCOW", "MOTO", "MOTORCYCLES",
"MOV", "MOVIE", "MOVISTAR", "MP", "MQ", "MR", "MS", "MSD", "MT", "MTN", "MTPC",
"MTR", "MU", "MUSEUM", "MUTUAL", "MV", "MW", "MX", "MY", "MZ", "NA", "NAB",
"NADEX", "NAGOYA", "NAME", "NATIONWIDE", "NATURA", "NAVY", "NBA", "NC", "NE",
"NEC", "NET", "NETBANK", "NETFLIX", "NETWORK", "NEUSTAR", "NEW", "NEWHOLLAND",
"NEWS", "NEXT", "NEXTDIRECT", "NEXUS", "NF", "NFL", "NG", "NGO", "NHK", "NI",
"NICO", "NIKE", "NIKON", "NINJA", "NISSAN", "NISSAY", "NL", "NO", "NOKIA",
"NORTHWESTERNMUTUAL", "NORTON", "NOW", "NOWRUZ", "NOWTV", "NP", "NR", "NRA",
"NRW", "NTT", "NU", "NYC", "NZ", "OBI", "OBSERVER", "OFF", "OFFICE", "OKINAWA",
"OLAYAN", "OLAYANGROUP", "OLDNAVY", "OLLO", "OM", "OMEGA", "ONE", "ONG", "ONL",
"ONLINE", "ONYOURSIDE", "OOO", "OPEN", "ORACLE", "ORANGE", "ORG", "ORGANIC",
"ORIENTEXPRESS", "ORIGINS", "OSAKA", "OTSUKA", "OTT", "OVH", "PA", "PAGE",
"PAMPEREDCHEF", "PANASONIC", "PANERAI", "PARIS", "PARS", "PARTNERS", "PARTS",
"PARTY", "PASSAGENS", "PAY", "PCCW", "PE", "PET", "PF", "PFIZER", "PG", "PH",
"PHARMACY", "PHILIPS", "PHONE", "PHOTO", "PHOTOGRAPHY", "PHOTOS", "PHYSIO",
"PIAGET", "PICS", "PICTET", "PICTURES", "PID", "PIN", "PING", "PINK",
"PIONEER", "PIZZA", "PK", "PL", "PLACE", "PLAY", "PLAYSTATION", "PLUMBING",
"PLUS", "PM", "PN", "PNC", "POHL", "POKER", "POLITIE", "PORN", "POST", "PR",
"PRAMERICA", "PRAXI", "PRESS", "PRIME", "PRO", "PROD", "PRODUCTIONS", "PROF",
"PROGRESSIVE", "PROMO", "PROPERTIES", "PROPERTY", "PROTECTION", "PRU",
"PRUDENTIAL", "PS", "PT", "PUB", "PW", "PWC", "PY", "QA", "QPON", "QUEBEC",
"QUEST", "QVC", "RACING", "RADIO", "RAID", "RE", "READ", "REALESTATE",
"REALTOR", "REALTY", "RECIPES", "RED", "REDSTONE", "REDUMBRELLA", "REHAB",
"REISE", "REISEN", "REIT", "RELIANCE", "REN", "RENT", "RENTALS", "REPAIR",
"REPORT", "REPUBLICAN", "REST", "RESTAURANT", "REVIEW", "REVIEWS", "REXROTH",
"RICH", "RICHARDLI", "RICOH", "RIGHTATHOME", "RIL", "RIO", "RIP", "RMIT", "RO",
"ROCHER", "ROCKS", "RODEO", "ROGERS", "ROOM", "RS", "RSVP", "RU", "RUHR",
"RUN", "RW", "RWE", "RYUKYU", "SA", "SAARLAND", "SAFE", "SAFETY", "SAKURA",
"SALE", "SALON", "SAMSCLUB", "SAMSUNG", "SANDVIK", "SANDVIKCOROMANT", "SANOFI",
"SAP", "SAPO", "SARL", "SAS", "SAVE", "SAXO", "SB", "SBI", "SBS", "SC", "SCA",
"SCB", "SCHAEFFLER", "SCHMIDT", "SCHOLARSHIPS", "SCHOOL", "SCHULE", "SCHWARZ",
"SCIENCE", "SCJOHNSON", "SCOR", "SCOT", "SD", "SE", "SEAT", "SECURE",
"SECURITY", "SEEK", "SELECT", "SENER", "SERVICES", "SES", "SEVEN", "SEW",
"SEX", "SEXY", "SFR", "SG", "SH", "SHANGRILA", "SHARP", "SHAW", "SHELL",
"SHIA", "SHIKSHA", "SHOES", "SHOP", "SHOPPING", "SHOUJI", "SHOW", "SHOWTIME",
"SHRIRAM", "SI", "SILK", "SINA", "SINGLES", "SITE", "SJ", "SK", "SKI", "SKIN",
"SKY", "SKYPE", "SL", "SLING", "SM", "SMART", "SMILE", "SN", "SNCF", "SO",
"SOCCER", "SOCIAL", "SOFTBANK", "SOFTWARE", "SOHU", "SOLAR", "SOLUTIONS",
"SONG", "SONY", "SOY", "SPACE", "SPIEGEL", "SPOT", "SPREADBETTING", "SR",
"SRL", "SRT", "ST", "STADA", "STAPLES", "STAR", "STARHUB", "STATEBANK",
"STATEFARM", "STATOIL", "STC", "STCGROUP", "STOCKHOLM", "STORAGE", "STORE",
"STREAM", "STUDIO", "STUDY", "STYLE", "SU", "SUCKS", "SUPPLIES", "SUPPLY",
"SUPPORT", "SURF", "SURGERY", "SUZUKI", "SV", "SWATCH", "SWIFTCOVER", "SWISS",
"SX", "SY", "SYDNEY", "SYMANTEC", "SYSTEMS", "SZ", "TAB", "TAIPEI", "TALK",
"TAOBAO", "TARGET", "TATAMOTORS", "TATAR", "TATTOO", "TAX", "TAXI", "TC",
"TCI", "TD", "TDK", "TEAM", "TECH", "TECHNOLOGY", "TEL", "TELECITY",
"TELEFONICA", "TEMASEK", "TENNIS", "TEVA", "TF", "TG", "TH", "THD", "THEATER",
"THEATRE", "TIAA", "TICKETS", "TIENDA", "TIFFANY", "TIPS", "TIRES", "TIROL",
"TJ", "TJMAXX", "TJX", "TK", "TKMAXX", "TL", "TM", "TMALL", "TN", "TO",
"TODAY", "TOKYO", "TOOLS", "TOP", "TORAY", "TOSHIBA", "TOTAL", "TOURS", "TOWN",
"TOYOTA", "TOYS", "TR", "TRADE", "TRADING", "TRAINING", "TRAVEL",
"TRAVELCHANNEL", "TRAVELERS", "TRAVELERSINSURANCE", "TRUST", "TRV", "TT",
"TUBE", "TUI", "TUNES", "TUSHU", "TV", "TVS", "TW", "TZ", "UA", "UBANK", "UBS",
"UCONNECT", "UG", "UK", "UNICOM", "UNIVERSITY", "UNO", "UOL", "UPS", "US",
"UY", "UZ", "VA", "VACATIONS", "VANA", "VANGUARD", "VC", "VE", "VEGAS",
"VENTURES", "VERISIGN", "VERSICHERUNG", "VET", "VG", "VI", "VIAJES", "VIDEO",
"VIG", "VIKING", "VILLAS", "VIN", "VIP", "VIRGIN", "VISA", "VISION", "VISTA",
"VISTAPRINT", "VIVA", "VIVO", "VLAANDEREN", "VN", "VODKA", "VOLKSWAGEN",
"VOLVO", "VOTE", "VOTING", "VOTO", "VOYAGE", "VU", "VUELOS", "WALES",
"WALMART", "WALTER", "WANG", "WANGGOU", "WARMAN", "WATCH", "WATCHES",
"WEATHER", "WEATHERCHANNEL", "WEBCAM", "WEBER", "WEBSITE", "WED", "WEDDING",
"WEIBO", "WEIR", "WF", "WHOSWHO", "WIEN", "WIKI", "WILLIAMHILL", "WIN",
"WINDOWS", "WINE", "WINNERS", "WME", "WOLTERSKLUWER", "WOODSIDE", "WORK",
"WORKS", "WORLD", "WOW", "WS", "WTC", "WTF", "XBOX", "XEROX", "XFINITY",
"XIHUAN", "XIN", "XN--11B4C3D", "XN--1CK2E1B", "XN--1QQW23A", "XN--30RR7Y",
"XN--3BST00M", "XN--3DS443G", "XN--3E0B707E", "XN--3OQ18VL8PN36A",
"XN--3PXU8K", "XN--42C2D9A", "XN--45BRJ9C", "XN--45Q11C", "XN--4GBRIM",
"XN--54B7FTA0CC", "XN--55QW42G", "XN--55QX5D", "XN--5SU34J936BGSG",
"XN--5TZM5G", "XN--6FRZ82G", "XN--6QQ986B3XL", "XN--80ADXHKS", "XN--80AO21A",
"XN--80AQECDR1A", "XN--80ASEHDB", "XN--80ASWG", "XN--8Y0A063A", "XN--90A3AC",
"XN--90AE", "XN--90AIS", "XN--9DBQ2A", "XN--9ET52U", "XN--9KRT00A",
"XN--B4W605FERD", "XN--BCK1B9A5DRE4C", "XN--C1AVG", "XN--C2BR7G",
"XN--CCK2B3B", "XN--CG4BKI", "XN--CLCHC0EA0B2G2A9GCD", "XN--CZR694B",
"XN--CZRS0T", "XN--CZRU2D", "XN--D1ACJ3B", "XN--D1ALF", "XN--E1A4C",
"XN--ECKVDTC9D", "XN--EFVY88H", "XN--ESTV75G", "XN--FCT429K", "XN--FHBEI",
"XN--FIQ228C5HS", "XN--FIQ64B", "XN--FIQS8S", "XN--FIQZ9S", "XN--FJQ720A",
"XN--FLW351E", "XN--FPCRJ9C3D", "XN--FZC2C9E2C", "XN--FZYS8D69UVGM",
"XN--G2XX48C", "XN--GCKR3F0F", "XN--GECRJ9C", "XN--GK3AT1E", "XN--H2BRJ9C",
"XN--HXT814E", "XN--I1B6B1A6A2E", "XN--IMR513N", "XN--IO0A7I", "XN--J1AEF",
"XN--J1AMH", "XN--J6W193G", "XN--JLQ61U9W7B", "XN--JVR189M", "XN--KCRX77D1X4A",
"XN--KPRW13D", "XN--KPRY57D", "XN--KPU716F", "XN--KPUT3I", "XN--L1ACC",
"XN--LGBBAT1AD8J", "XN--MGB9AWBF", "XN--MGBA3A3EJT", "XN--MGBA3A4F16A",
"XN--MGBA7C0BBN0A", "XN--MGBAAM7A8H", "XN--MGBAB2BD", "XN--MGBAI9AZGQP6J",
"XN--MGBAYH7GPA", "XN--MGBB9FBPOB", "XN--MGBBH1A71E", "XN--MGBC0A9AZCG",
"XN--MGBCA7DZDO", "XN--MGBERP4A5D4AR", "XN--MGBI4ECEXP", "XN--MGBPL2FH",
"XN--MGBT3DHD", "XN--MGBTX2B", "XN--MGBX4CD0AB", "XN--MIX891F", "XN--MK1BU44C",
"XN--MXTQ1M", "XN--NGBC5AZD", "XN--NGBE9E0A", "XN--NODE", "XN--NQV7F",
"XN--NQV7FS00EMA", "XN--NYQY26A", "XN--O3CW4H", "XN--OGBPF8FL", "XN--P1ACF",
"XN--P1AI", "XN--PBT977C", "XN--PGBS0DH", "XN--PSSY2U", "XN--Q9JYB4C",
"XN--QCKA1PMC", "XN--QXAM", "XN--RHQV96G", "XN--ROVU88B", "XN--S9BRJ9C",
"XN--SES554G", "XN--T60B56A", "XN--TCKWE", "XN--TIQ49XQYJ", "XN--UNUP4Y",
"XN--VERMGENSBERATER-CTB", "XN--VERMGENSBERATUNG-PWB", "XN--VHQUV",
"XN--VUQ861B", "XN--W4R85EL8FHU5DNRA", "XN--W4RS40L", "XN--WGBH1C",
"XN--WGBL6A", "XN--XHQ521B", "XN--XKC2AL3HYE2A", "XN--XKC2DL3A5EE0H",
"XN--Y9A3AQ", "XN--YFRO4I67O", "XN--YGBI2AMMX", "XN--ZFR164B", "XPERIA", "XXX",
"XYZ", "YACHTS", "YAHOO", "YAMAXUN", "YANDEX", "YE", "YODOBASHI", "YOGA",
"YOKOHAMA", "YOU", "YOUTUBE", "YT", "YUN", "ZA", "ZAPPOS", "ZARA", "ZERO",
"ZIP", "ZIPPO", "ZM", "ZONE", "ZUERICH", "ZW"}
};



enum class Section{ Enabled, Name, Trigger, URL, Count };

std::map<QString,QIcon> iconCache;

struct SearchEngine {
    bool    enabled;
    QString name;
    QString trigger;
    QString iconPath;
    QString url;
};

std::vector<SearchEngine> defaultSearchEngines = {
    {true, "Google",        "gg ",  ":google",    "https://www.google.com/search?q=%s"},
    {true, "Youtube",       "yt ",  ":youtube",   "https://www.youtube.com/results?search_query=%s"},
    {true, "Amazon",        "ama ", ":amazon",    "http://www.amazon.com/s/?field-keywords=%s"},
    {true, "Ebay",          "eb ",  ":ebay",      "http://www.ebay.com/sch/i.html?_nkw=%s"},
    {true, "GitHub",        "gh ",  ":github",    "https://github.com/search?utf8=✓&q=%s"},
    {true, "Wikipedia",     "wp ",  ":wikipedia", "https://wikipedia.org/w/index.php?search=%s"},
    {true, "Wolfram Alpha", "=",    ":wolfram",   "https://www.wolframalpha.com/input/?i=%s"}
};

shared_ptr<Core::Item> buildWebsearchItem(const SearchEngine &se, const QString &searchterm) {

    QString urlString = QString(se.url).replace("%s", QUrl::toPercentEncoding(searchterm));
    QUrl url = QUrl(urlString);
    QString desc = QString("Start %1 search in your browser").arg(se.name);

    std::shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
    action->setText(desc);
    action->setAction([=](){ QDesktopServices::openUrl(url); });

    std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>(se.name);
    item->setText(se.name);
    item->setSubtext(desc);
    item->setIconPath(se.iconPath);

    item->setActions({action});

    return item;
}
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Websearch::WebsearchPrivate
{
public:
    WebsearchPrivate(Extension *q) : q(q) {}

    bool deserialize();
    bool serialize();
    void restoreDefaults();

    QPointer<ConfigWidget> widget;
    std::vector<SearchEngine> searchEngines;

    Extension *q;
};



/** ***************************************************************************/
bool Websearch::WebsearchPrivate::deserialize() {

    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
               .filePath(QString("%1.json").arg(q->Core::Extension::id)));

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << qPrintable(QString("Could not open file: '%1'.").arg(file.fileName()));
        return false;
    }

    QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();

    SearchEngine searchEngine;
    for ( const QJsonValue& value : array) {
        QJsonObject object = value.toObject();
        searchEngine.enabled  = object["enabled"].toBool();
        searchEngine.name     = object["name"].toString();
        searchEngine.trigger  = object["trigger"].toString();
        searchEngine.iconPath = object["iconPath"].toString();
        searchEngine.url      = object["url"].toString();
        searchEngines.push_back(searchEngine);
    }

    return true;
}



/** ***************************************************************************/
bool Websearch::WebsearchPrivate::serialize() {

    QFile file(QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
               .filePath(QString("%1.json").arg(q->Core::Extension::id)));

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << qPrintable(QString("Could not open file: '%1'.").arg(file.fileName()));
        return false;
    }

    QJsonArray array;

    for ( const SearchEngine& searchEngine : searchEngines ) {
        QJsonObject object;
        object["name"]     = searchEngine.name;
        object["url"]      = searchEngine.url;
        object["trigger"]  = searchEngine.trigger;
        object["iconPath"] = searchEngine.iconPath;
        object["enabled"]  = searchEngine.enabled;
        array.append(object);
    }

    file.write(QJsonDocument(array).toJson());

    return true;
}



/** ***************************************************************************/
void Websearch::WebsearchPrivate::restoreDefaults() {
    /* Init std searches */
    searchEngines = defaultSearchEngines;
    serialize();

    if (!widget.isNull())
        widget->ui.tableView_searches->reset();
}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Websearch::Extension::Extension()
    : Core::Extension("org.albert.extension.websearch"),
      Core::QueryHandler(Core::Extension::id),
      d(new WebsearchPrivate(this)) {

    std::sort(validTlds.begin(), validTlds.end());

    QString writableLocation =  QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFile dataFile(QDir(writableLocation).filePath(QString("%1.dat").arg(Core::Extension::id)));
    QFile jsonFile(QDir(writableLocation).filePath(QString("%1.json").arg(Core::Extension::id)));

    // If there is an old file
    if (dataFile.exists()) {

        // Deserialize binary data
        if (dataFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
            qDebug() << "Porting websearches from old format";
            QDataStream in(&dataFile);
            quint64 size;
            in >> size;
            SearchEngine se;
            for (quint64 i = 0; i < size; ++i) {
                in >> se.enabled >> se.url >> se.name >> se.trigger >> se.iconPath;
                d->searchEngines.push_back(se);
            }
            dataFile.close();
        } else
            qWarning() << qPrintable(QString("Could not open file '%1'").arg(dataFile.fileName()));

        // Whatever remove it
        if ( !dataFile.remove() )
            qWarning() << qPrintable(QString("Could not remove file '%1'").arg(dataFile.fileName()));

        // Hmm what to do?

        // Serialize in json format
        d->serialize();

    } else if (!d->deserialize())
        d->restoreDefaults();
}



/** ***************************************************************************/
Websearch::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Websearch::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()){
        d->widget = new ConfigWidget(parent);
        d->widget->ui.tableView_searches->setModel(this);
        // TODO Fix all data() if least supported Qt supports its omittance
        connect(d->widget.data(), &ConfigWidget::restoreDefaults,
                std::bind(&WebsearchPrivate::restoreDefaults, std::ref(d)));
    }
    return d->widget;
}



/** ***************************************************************************/
void Websearch::Extension::handleQuery(Core::Query * query) {

    QUrl url = QUrl::fromUserInput(query->searchTerm());

    // Check syntax and TLD validity
    if ( url.isValid() && // Check syntax
         QRegularExpression(R"R(\S+\.\S+$)R").match(url.host()).hasMatch() &&  // Check if not an emty tld
         std::binary_search(validTlds.begin(), validTlds.end(), // Check tld validiy
                            url.topLevelDomain().mid(1).toUpper().toLocal8Bit().constData()) ) {

        shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
        action->setText("Open URL");
        action->setAction([url](){
            QDesktopServices::openUrl(url);
        });

        std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>("valid_url");
        item->setText(QString("Open url in browser"));
        item->setSubtext(QString("Visit %1'").arg(url.authority()));
        item->setCompletionString(query->searchTerm());
        QString icon = XdgIconLookup::iconPath("www");
        if (icon.isEmpty())
            icon = XdgIconLookup::iconPath("web-browser");
        if (icon.isEmpty())
            icon = XdgIconLookup::iconPath("emblem-web");
        if (icon.isEmpty())
            icon = ":favicon";
        item->setIconPath(icon);
        item->setActions({action});

        query->addMatch(item, SHRT_MAX);
    }

    for (const SearchEngine &se : d->searchEngines)
        if (query->searchTerm().startsWith(se.trigger))
            query->addMatch(buildWebsearchItem(se, query->searchTerm().mid(se.trigger.size())), SHRT_MAX);
}



/** ***************************************************************************/
vector<shared_ptr<Core::Item>> Websearch::Extension::fallbacks(const QString & searchterm) {
    vector<shared_ptr<Core::Item>> res;
    for (const SearchEngine &se : d->searchEngines)
        if (se.enabled)
            res.push_back(buildWebsearchItem(se, searchterm));
    return res;
}



/** ***************************************************************************/
int Websearch::Extension::rowCount(const QModelIndex &) const {
    return static_cast<int>(d->searchEngines.size());
}



/** ***************************************************************************/
int Websearch::Extension::columnCount(const QModelIndex &) const {
    return static_cast<int>(Section::Count);
}



/** ***************************************************************************/
QVariant Websearch::Extension::headerData(int section, Qt::Orientation orientation, int role) const {
    // No sanity check necessary since
    if ( section<0 || static_cast<int>(Section::Count)<=section )
        return QVariant();


    if (orientation == Qt::Horizontal){
        switch (static_cast<Section>(section)) {
        case Section::Enabled:{
            switch (role) {
            case Qt::ToolTipRole: return "Enables the searchengine as fallback.";
            default: return QVariant();
            }
        }
        case Section::Name:{
            switch (role) {
            case Qt::DisplayRole: return "Name";
            case Qt::ToolTipRole: return "The name of the searchengine.";
            default: return QVariant();
            }

        }
        case Section::Trigger:{
            switch (role) {
            case Qt::DisplayRole: return "Trigger";
            case Qt::ToolTipRole: return "The term that triggers this searchengine.";
            default: return QVariant();
            }

        }
        case Section::URL:{
            switch (role) {
            case Qt::DisplayRole: return "URL";
            case Qt::ToolTipRole: return "The URL of this searchengine. %s will be replaced by your searchterm.";
            default: return QVariant();
            }

        }
        default: return QVariant();
        }
    }
    return QVariant();
}



/** ***************************************************************************/
QVariant Websearch::Extension::data(const QModelIndex &index, int role) const {
    if (!index.isValid()
            || index.row() >= static_cast<int>(d->searchEngines.size())
            || index.column() >= static_cast<int>(static_cast<int>(Section::Count)))
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return d->searchEngines[index.row()].name;
        case Section::Trigger:  return d->searchEngines[index.row()].trigger;
        case Section::URL:  return d->searchEngines[index.row()].url;
        default: return QVariant();
        }
    }
    case Qt::EditRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:  return d->searchEngines[index.row()].name;
        case Section::Trigger:  return d->searchEngines[index.row()].trigger;
        case Section::URL:  return d->searchEngines[index.row()].url;
        default: return QVariant();
        }
    }
    case Qt::DecorationRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Name:{
            // Resizing request thounsands of repaints. Creating an icon for
            // ever paint event is to expensive. Therefor maintain an icon cache
            QString &iconPath = d->searchEngines[index.row()].iconPath;
            decltype(iconCache)::iterator it = iconCache.find(iconPath);
            if ( it != iconCache.end() )
                return it->second;
            return iconCache.emplace(iconPath, QIcon(iconPath)).second;
        }
        default: return QVariant();
        }
    }
    case Qt::ToolTipRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:  return "Check to use as fallback";
        default: return "Double click to edit";
        }
    }
    case Qt::CheckStateRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:  return (d->searchEngines[index.row()].enabled)?Qt::Checked:Qt::Unchecked;
        default: return QVariant();
        }
    }
    default:
        return QVariant();
    }
}



/** ***************************************************************************/
bool Websearch::Extension::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid()
            || index.row() >= static_cast<int>(d->searchEngines.size())
            || index.column() >= static_cast<int>(static_cast<int>(Section::Count)))
        return false;

    switch (role) {
    case Qt::EditRole: {
        if ( !value.canConvert(QMetaType::QString) )
            return false;
        QString s = value.toString();
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:
            return false;
        case Section::Name:
            d->searchEngines[index.row()].name = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            d->serialize();
            return true;
        case Section::Trigger:
            d->searchEngines[index.row()].trigger = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            d->serialize();
            return true;
        case Section::URL:
            d->searchEngines[index.row()].url = s;
            dataChanged(index, index, QVector<int>({Qt::DisplayRole}));
            d->serialize();
            return true;
        default:
            return false;
        }
    }
    case Qt::CheckStateRole: {
        switch (static_cast<Section>(index.column())) {
        case Section::Enabled:
            d->searchEngines[index.row()].enabled = value.toBool();
            dataChanged(index, index, QVector<int>({Qt::CheckStateRole}));
            d->serialize();
            return true;
        default:
            return false;
        }
    }
    case Qt::DecorationRole: {
        QFileInfo fileInfo(value.toString());
        QString newFilePath;
        uint i = 0;
        do {
        // Build the new path in cache dir
        newFilePath = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
                .filePath(QString("%1-%2.%3")
                            .arg(d->searchEngines[index.row()].name)
                            .arg(i++)
                            .arg(fileInfo.suffix()));
        // Copy the file into cache dir
        } while (!QFile::copy(fileInfo.filePath(), newFilePath));
        // Set the copied file as icon
        d->searchEngines[index.row()].iconPath = newFilePath;
        dataChanged(index, index, QVector<int>({Qt::DecorationRole}));
        d->serialize();
        iconCache.clear();
        return true;
    }
    default:
        return false;
    }
    return false;
}



/** ***************************************************************************/
Qt::ItemFlags Websearch::Extension::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    switch (static_cast<Section>(index.column())) {
    case Section::Enabled:
        return Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable;
    default:
        return Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable;
    }
}



/** ***************************************************************************/
bool Websearch::Extension::insertRows(int position, int rows, const QModelIndex &) {
    if (position<0 || rows<1 || static_cast<int>(d->searchEngines.size())<position)
        return false;

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = position; row < position + rows; ++row){
        d->searchEngines.insert(d->searchEngines.begin() + row,
                              SearchEngine({false,
                                            "<name>",
                                            "<trigger>",
                                            ":default",
                                            "<http://url/containing/the/?query=%s>"}));
    }
    endInsertRows();

    d->serialize();

    return true;
}



/** ***************************************************************************/
bool Websearch::Extension::removeRows(int position, int rows, const QModelIndex &) {
    if (position<0 || rows<1 || static_cast<int>(d->searchEngines.size())<position+rows)
        return false;

    beginRemoveRows(QModelIndex(), position, position + rows-1);
    d->searchEngines.erase(d->searchEngines.begin()+position,d->searchEngines.begin()+(position+rows));
    endRemoveRows();

    d->serialize();

    return true;
}



/** ***************************************************************************/
bool Websearch::Extension::moveRows(const QModelIndex &src, int srcRow, int cnt, const QModelIndex &dst, int dstRow) {
    if (srcRow<0 || cnt<1 || dstRow<0
            || static_cast<int>(d->searchEngines.size())<srcRow+cnt-1
            || static_cast<int>(d->searchEngines.size())<dstRow
            || ( srcRow<=dstRow && dstRow<srcRow+cnt) ) // If its inside the source do nothing
        return false;

    vector<SearchEngine> tmp;
    beginMoveRows(src, srcRow, srcRow+cnt-1, dst, dstRow);
    tmp.insert(tmp.end(), make_move_iterator(d->searchEngines.begin()+srcRow), make_move_iterator(d->searchEngines.begin() + srcRow+cnt));
    d->searchEngines.erase(d->searchEngines.begin()+srcRow, d->searchEngines.begin() + srcRow+cnt);
    const size_t finalDst = dstRow > srcRow ? dstRow - cnt : dstRow;
    d->searchEngines.insert(d->searchEngines.begin()+finalDst , make_move_iterator(tmp.begin()), make_move_iterator(tmp.end()));
    endMoveRows();

    d->serialize();

    return true;
}

