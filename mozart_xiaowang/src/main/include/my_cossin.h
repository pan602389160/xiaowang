#ifndef COS_SIN_H_
#define COS_SIN_H_
#include "my_fft.h"
const complex W[DECORDE_SIZE]={
{1.000000,0.000000},{0.999981,0.006100},{0.999926,0.012200},{0.999833,0.018299},{0.999702,0.024398},{0.999535,0.030495},{0.999330,0.036592},{0.999088,0.042687},
{0.998810,0.048781},{0.998493,0.054872},{0.998140,0.060962},{0.997750,0.067050},{0.997322,0.073135},{0.996857,0.079217},{0.996356,0.085296},{0.995817,0.091372},
{0.995241,0.097445},{0.994628,0.103514},{0.993978,0.109580},{0.993291,0.115641},{0.992567,0.121698},{0.991806,0.127750},{0.991009,0.133798},{0.990174,0.139840},
{0.989303,0.145878},{0.988394,0.151910},{0.987449,0.157936},{0.986468,0.163956},{0.985449,0.169971},{0.984394,0.175979},{0.983302,0.181980},{0.982174,0.187975},
{0.981009,0.193963},{0.979807,0.199943},{0.978570,0.205916},{0.977295,0.211882},{0.975985,0.217839},{0.974638,0.223789},{0.973254,0.229730},{0.971835,0.235662},
{0.970379,0.241586},{0.968888,0.247501},{0.967360,0.253406},{0.965796,0.259303},{0.964196,0.265189},{0.962561,0.271066},{0.960889,0.276932},{0.959182,0.282788},
{0.957439,0.288634},{0.955661,0.294469},{0.953847,0.300293},{0.951997,0.306106},{0.950112,0.311907},{0.948192,0.317697},{0.946237,0.323475},{0.944246,0.329241},
{0.942220,0.334995},{0.940159,0.340736},{0.938063,0.346465},{0.935932,0.352181},{0.933766,0.357883},{0.931566,0.363573},{0.929331,0.369248},{0.927061,0.374910},
{0.924757,0.380558},{0.922418,0.386192},{0.920045,0.391812},{0.917638,0.397417},{0.915197,0.403007},{0.912722,0.408582},{0.910212,0.414142},{0.907669,0.419687},
{0.905092,0.425216},{0.902481,0.430729},{0.899837,0.436226},{0.897160,0.441707},{0.894448,0.447171},{0.891704,0.452619},{0.888927,0.458050},{0.886116,0.463464},
{0.883272,0.468860},{0.880396,0.474239},{0.877487,0.479601},{0.874545,0.484945},{0.871570,0.490270},{0.868564,0.495578},{0.865524,0.500867},{0.862453,0.506137},
{0.859350,0.511389},{0.856214,0.516621},{0.853047,0.521834},{0.849848,0.527028},{0.846617,0.532203},{0.843355,0.537357},{0.840061,0.542491},{0.836737,0.547606},
{0.833381,0.552700},{0.829994,0.557773},{0.826576,0.562825},{0.823127,0.567857},{0.819648,0.572867},{0.816138,0.577857},{0.812598,0.582824},{0.809028,0.587770},
{0.805427,0.592694},{0.801797,0.597596},{0.798137,0.602476},{0.794447,0.607334},{0.790727,0.612168},{0.786978,0.616980},{0.783200,0.621769},{0.779393,0.626535},
{0.775557,0.631278},{0.771691,0.635997},{0.767797,0.640693},{0.763875,0.645364},{0.759924,0.650012},{0.755945,0.654635},{0.751938,0.659234},{0.747902,0.663809},
{0.743839,0.668359},{0.739748,0.672884},{0.735630,0.677384},{0.731484,0.681858},{0.727311,0.686308},{0.723111,0.690731},{0.718885,0.695129},{0.714631,0.699502},
{0.710351,0.703848},{0.706044,0.708168},{0.701711,0.712462},{0.697352,0.716729},{0.692967,0.720969},{0.688556,0.725183},{0.684120,0.729370},{0.679658,0.733529},
{0.675171,0.737661},{0.670659,0.741766},{0.666121,0.745843},{0.661559,0.749893},{0.656973,0.753914},{0.652362,0.757908},{0.647726,0.761873},{0.643067,0.765810},
{0.638384,0.769718},{0.633676,0.773598},{0.628946,0.777449},{0.624192,0.781271},{0.619414,0.785064},{0.614614,0.788828},{0.609791,0.792563},{0.604945,0.796268},
{0.600076,0.799943},{0.595185,0.803588},{0.590272,0.807204},{0.585338,0.810790},{0.580381,0.814345},{0.575403,0.817870},{0.570403,0.821365},{0.565382,0.824829},
{0.560340,0.828263},{0.555277,0.831665},{0.550194,0.835037},{0.545090,0.838378},{0.539966,0.841687},{0.534821,0.844965},{0.529657,0.848212},{0.524473,0.851427},
{0.519270,0.854610},{0.514047,0.857762},{0.508805,0.860882},{0.503544,0.863969},{0.498265,0.867025},{0.492967,0.870048},{0.487650,0.873039},{0.482316,0.875997},
{0.476963,0.878923},{0.471593,0.881816},{0.466205,0.884677},{0.460800,0.887504},{0.455378,0.890298},{0.449938,0.893060},{0.444482,0.895788},{0.439010,0.898482},
{0.433521,0.901143},{0.428016,0.903771},{0.422495,0.906365},{0.416958,0.908926},{0.411406,0.911452},{0.405839,0.913945},{0.400256,0.916403},{0.394659,0.918828},
{0.389047,0.921218},{0.383420,0.923574},{0.377779,0.925896},{0.372124,0.928183},{0.366455,0.930436},{0.360773,0.932654},{0.355077,0.934837},{0.349368,0.936986},
{0.343646,0.939099},{0.337911,0.941178},{0.332163,0.943222},{0.326404,0.945230},{0.320632,0.947204},{0.314848,0.949142},{0.309052,0.951045},{0.303245,0.952913},
{0.297427,0.954745},{0.291597,0.956541},{0.285757,0.958302},{0.279906,0.960027},{0.274045,0.961717},{0.268173,0.963371},{0.262292,0.964989},{0.256400,0.966571},
{0.250500,0.968117},{0.244590,0.969627},{0.238670,0.971101},{0.232742,0.972538},{0.226805,0.973940},{0.220860,0.975305},{0.214907,0.976635},{0.208945,0.977927},
{0.202976,0.979184},{0.196999,0.980404},{0.191015,0.981587},{0.185024,0.982734},{0.179026,0.983844},{0.173021,0.984918},{0.167010,0.985955},{0.160993,0.986956},
{0.154969,0.987919},{0.148940,0.988846},{0.142905,0.989736},{0.136865,0.990590},{0.130820,0.991406},{0.124770,0.992186},{0.118716,0.992928},{0.112657,0.993634},
{0.106593,0.994303},{0.100526,0.994934},{0.094455,0.995529},{0.088381,0.996087},{0.082303,0.996607},{0.076222,0.997091},{0.070139,0.997537},{0.064052,0.997947},
{0.057964,0.998319},{0.051873,0.998654},{0.045780,0.998952},{0.039686,0.999212},{0.033590,0.999436},{0.027493,0.999622},{0.021395,0.999771},{0.015296,0.999883},
{0.009196,0.999958},{0.003096,0.999995},{-0.003004,0.999995},{-0.009104,0.999959},{-0.015203,0.999884},{-0.021302,0.999773},{-0.027400,0.999625},{-0.033497,0.999439},
{-0.039593,0.999216},{-0.045688,0.998956},{-0.051781,0.998658},{-0.057871,0.998324},{-0.063960,0.997952},{-0.070046,0.997544},{-0.076130,0.997098},{-0.082211,0.996615},
{-0.088289,0.996095},{-0.094363,0.995538},{-0.100434,0.994944},{-0.106501,0.994313},{-0.112565,0.993644},{-0.118624,0.992939},{-0.124678,0.992197},{-0.130728,0.991418},
{-0.136774,0.990602},{-0.142814,0.989750},{-0.148848,0.988860},{-0.154878,0.987934},{-0.160901,0.986971},{-0.166919,0.985971},{-0.172930,0.984934},{-0.178935,0.983861},
{-0.184933,0.982751},{-0.190924,0.981605},{-0.196908,0.980422},{-0.202885,0.979202},{-0.208855,0.977947},{-0.214816,0.976654},{-0.220770,0.975326},{-0.226715,0.973961},
{-0.232652,0.972560},{-0.238580,0.971123},{-0.244500,0.969649},{-0.250410,0.968140},{-0.256311,0.966594},{-0.262202,0.965013},{-0.268084,0.963396},{-0.273956,0.961742},
{-0.279817,0.960053},{-0.285668,0.958329},{-0.291509,0.956568},{-0.297338,0.954772},{-0.303157,0.952941},{-0.308964,0.951074},{-0.314760,0.949171},{-0.320544,0.947234},
{-0.326316,0.945261},{-0.332076,0.943253},{-0.337824,0.941209},{-0.343559,0.939131},{-0.349281,0.937018},{-0.354990,0.934870},{-0.360686,0.932687},{-0.366369,0.930470},
{-0.372038,0.928217},{-0.377693,0.925931},{-0.383334,0.923610},{-0.388961,0.921254},{-0.394574,0.918864},{-0.400171,0.916440},{-0.405754,0.913982},{-0.411322,0.911490},
{-0.416874,0.908964},{-0.422411,0.906404},{-0.427932,0.903811},{-0.433437,0.901184},{-0.438927,0.898523},{-0.444399,0.895829},{-0.449856,0.893101},{-0.455295,0.890341},
{-0.460718,0.887547},{-0.466123,0.884720},{-0.471511,0.881860},{-0.476882,0.878967},{-0.482235,0.876042},{-0.487569,0.873084},{-0.492886,0.870094},{-0.498184,0.867071},
{-0.503464,0.864016},{-0.508725,0.860929},{-0.513968,0.857810},{-0.519191,0.854658},{-0.524394,0.851476},{-0.529579,0.848261},{-0.534743,0.845015},{-0.539888,0.841737},
{-0.545012,0.838428},{-0.550116,0.835088},{-0.555200,0.831717},{-0.560263,0.828315},{-0.565306,0.824882},{-0.570327,0.821418},{-0.575327,0.817924},{-0.580305,0.814399},
{-0.585262,0.810844},{-0.590198,0.807259},{-0.595111,0.803644},{-0.600002,0.799998},{-0.604871,0.796324},{-0.609717,0.792619},{-0.614541,0.788885},{-0.619341,0.785122},
{-0.624119,0.781329},{-0.628874,0.777508},{-0.633605,0.773657},{-0.638312,0.769778},{-0.642996,0.765870},{-0.647656,0.761933},{-0.652291,0.757968},{-0.656903,0.753975},
{-0.661490,0.749954},{-0.666052,0.745905},{-0.670590,0.741828},{-0.675103,0.737724},{-0.679590,0.733592},{-0.684052,0.729433},{-0.688489,0.725247},{-0.692900,0.721033},
{-0.697286,0.716793},{-0.701645,0.712527},{-0.705978,0.708233},{-0.710285,0.703914},{-0.714566,0.699568},{-0.718820,0.695196},{-0.723047,0.690798},{-0.727248,0.686375},
{-0.731421,0.681926},{-0.735567,0.677452},{-0.739686,0.672952},{-0.743777,0.668428},{-0.747841,0.663878},{-0.751877,0.659304},{-0.755884,0.654705},{-0.759864,0.650082},
{-0.763815,0.645435},{-0.767738,0.640764},{-0.771632,0.636069},{-0.775498,0.631350},{-0.779335,0.626608},{-0.783143,0.621842},{-0.786921,0.617053},{-0.790671,0.612242},
{-0.794391,0.607407},{-0.798081,0.602550},{-0.801742,0.597671},{-0.805373,0.592769},{-0.808973,0.587845},{-0.812544,0.582900},{-0.816085,0.577932},{-0.819595,0.572943},
{-0.823075,0.567933},{-0.826524,0.562902},{-0.829942,0.557850},{-0.833329,0.552777},{-0.836686,0.547683},{-0.840011,0.542569},{-0.843305,0.537435},{-0.846568,0.532281},
{-0.849799,0.527107},{-0.852998,0.521913},{-0.856166,0.516701},{-0.859302,0.511468},{-0.862406,0.506217},{-0.865478,0.500947},{-0.868518,0.495658},{-0.871525,0.490351},
{-0.874500,0.485026},{-0.877442,0.479682},{-0.880352,0.474321},{-0.883229,0.468942},{-0.886073,0.463546},{-0.888884,0.458132},{-0.891662,0.452701},{-0.894407,0.447254},
{-0.897119,0.441790},{-0.899797,0.436309},{-0.902442,0.430812},{-0.905053,0.425299},{-0.907630,0.419771},{-0.910174,0.414226},{-0.912684,0.408667},{-0.915160,0.403092},
{-0.917601,0.397502},{-0.920009,0.391897},{-0.922383,0.386278},{-0.924722,0.380644},{-0.927026,0.374996},{-0.929297,0.369334},{-0.931532,0.363659},{-0.933733,0.357970},
{-0.935899,0.352267},{-0.938031,0.346552},{-0.940127,0.340823},{-0.942189,0.335082},{-0.944215,0.329329},{-0.946207,0.323563},{-0.948163,0.317785},{-0.950084,0.311996},
{-0.951969,0.306194},{-0.953819,0.300382},{-0.955634,0.294558},{-0.957413,0.288723},{-0.959156,0.282877},{-0.960864,0.277021},{-0.962536,0.271155},{-0.964172,0.265278},
{-0.965772,0.259392},{-0.967336,0.253496},{-0.968865,0.247591},{-0.970357,0.241676},{-0.971813,0.235752},{-0.973233,0.229820},{-0.974617,0.223879},{-0.975964,0.217930},
{-0.977276,0.211972},{-0.978551,0.206007},{-0.979789,0.200034},{-0.980991,0.194054},{-0.982156,0.188066},{-0.983285,0.182071},{-0.984378,0.176070},{-0.985433,0.170062},
{-0.986452,0.164048},{-0.987435,0.158027},{-0.988380,0.152001},{-0.989289,0.145969},{-0.990161,0.139932},{-0.990996,0.133889},{-0.991795,0.127842},{-0.992556,0.121790},
{-0.993280,0.115733},{-0.993968,0.109672},{-0.994618,0.103606},{-0.995232,0.097537},{-0.995808,0.091465},{-0.996348,0.085389},{-0.996850,0.079309},{-0.997315,0.073227},
{-0.997743,0.067142},{-0.998134,0.061055},{-0.998488,0.054965},{-0.998805,0.048873},{-0.999085,0.042780},{-0.999327,0.036684},{-0.999532,0.030588},{-0.999700,0.024490},
{-0.999831,0.018392},{-0.999924,0.012292},{-0.999981,0.006193},{-1.000000,0.000093},{-0.999982,-0.006007},{-0.999927,-0.012107},{-0.999834,-0.018206},{-0.999705,-0.024305},
{-0.999538,-0.030403},{-0.999334,-0.036499},{-0.999092,-0.042594},{-0.998814,-0.048688},{-0.998498,-0.054780},{-0.998146,-0.060870},{-0.997756,-0.066957},{-0.997329,-0.073042},
{-0.996865,-0.079125},{-0.996364,-0.085204},{-0.995825,-0.091280},{-0.995250,-0.097353},{-0.994638,-0.103422},{-0.993988,-0.109487},{-0.993302,-0.115549},{-0.992578,-0.121606},
{-0.991818,-0.127658},{-0.991021,-0.133706},{-0.990187,-0.139748},{-0.989316,-0.145786},{-0.988408,-0.151818},{-0.987464,-0.157844},{-0.986483,-0.163865},{-0.985465,-0.169879},
{-0.984410,-0.175888},{-0.983319,-0.181889},{-0.982191,-0.187884},{-0.981027,-0.193872},{-0.979826,-0.199852},{-0.978589,-0.205826},{-0.977315,-0.211791},{-0.976005,-0.217749},
{-0.974658,-0.223698},{-0.973276,-0.229640},{-0.971857,-0.235572},{-0.970402,-0.241496},{-0.968911,-0.247411},{-0.967383,-0.253317},{-0.965820,-0.259213},{-0.964221,-0.265100},
{-0.962586,-0.270977},{-0.960915,-0.276843},{-0.959208,-0.282700},{-0.957466,-0.288545},{-0.955688,-0.294381},{-0.953875,-0.300205},{-0.952026,-0.306018},{-0.950141,-0.311819},
{-0.948222,-0.317609},{-0.946267,-0.323388},{-0.944276,-0.329154},{-0.942251,-0.334908},{-0.940190,-0.340649},{-0.938095,-0.346378},{-0.935965,-0.352094},{-0.933800,-0.357797},
{-0.931600,-0.363486},{-0.929365,-0.369162},{-0.927096,-0.374824},{-0.924792,-0.380473},{-0.922454,-0.386107},{-0.920082,-0.391727},{-0.917675,-0.397332},{-0.915234,-0.402922},
{-0.912759,-0.408497},{-0.910251,-0.414058},{-0.907708,-0.419602},{-0.905132,-0.425132},{-0.902521,-0.430645},{-0.899878,-0.436142},{-0.897200,-0.441623},{-0.894490,-0.447088},
{-0.891746,-0.452536},{-0.888969,-0.457967},{-0.886159,-0.463382},{-0.883316,-0.468778},{-0.880440,-0.474158},{-0.877531,-0.479520},{-0.874590,-0.484864},{-0.871616,-0.490190},
{-0.868609,-0.495497},{-0.865571,-0.500787},{-0.862500,-0.506057},{-0.859397,-0.511309},{-0.856262,-0.516542},{-0.853095,-0.521755},{-0.849897,-0.526950},{-0.846666,-0.532124},
{-0.843405,-0.537279},{-0.840112,-0.542414},{-0.836787,-0.547528},{-0.833432,-0.552622},{-0.830045,-0.557696},{-0.826628,-0.562749},{-0.823180,-0.567781},{-0.819701,-0.572792},
{-0.816192,-0.577781},{-0.812652,-0.582749},{-0.809082,-0.587695},{-0.805482,-0.592620},{-0.801852,-0.597522},{-0.798193,-0.602402},{-0.794503,-0.607260},{-0.790784,-0.612095},
{-0.787036,-0.616908},{-0.783258,-0.621697},{-0.779451,-0.626463},{-0.775615,-0.631206},{-0.771750,-0.635926},{-0.767857,-0.640621},{-0.763935,-0.645293},{-0.759984,-0.649941},
{-0.756006,-0.654565},{-0.751999,-0.659165},{-0.747964,-0.663740},{-0.743901,-0.668290},{-0.739811,-0.672815},{-0.735693,-0.677315},{-0.731548,-0.681790},{-0.727375,-0.686240},
{-0.723175,-0.690664},{-0.718949,-0.695063},{-0.714696,-0.699436},{-0.710416,-0.703782},{-0.706110,-0.708103},{-0.701777,-0.712397},{-0.697418,-0.716664},{-0.693034,-0.720905},
{-0.688623,-0.725119},{-0.684187,-0.729306},{-0.679726,-0.733466},{-0.675239,-0.737599},{-0.670727,-0.741704},{-0.666190,-0.745782},{-0.661629,-0.749831},{-0.657043,-0.753853},
{-0.652432,-0.757847},{-0.647797,-0.761813},{-0.643138,-0.765750},{-0.638455,-0.769659},{-0.633748,-0.773540},{-0.629018,-0.777391},{-0.624264,-0.781213},{-0.619487,-0.785007},
{-0.614687,-0.788771},{-0.609864,-0.792506},{-0.605018,-0.796211},{-0.600150,-0.799887},{-0.595260,-0.803533},{-0.590347,-0.807149},{-0.585413,-0.810735},{-0.580456,-0.814291},
{-0.575478,-0.817817},{-0.570479,-0.821312},{-0.565458,-0.824777},{-0.560417,-0.828211},{-0.555354,-0.831614},{-0.550271,-0.834986},{-0.545168,-0.838327},{-0.540044,-0.841637},
{-0.534900,-0.844916},{-0.529736,-0.848163},{-0.524552,-0.851378},{-0.519349,-0.854562},{-0.514127,-0.857714},{-0.508885,-0.860835},{-0.503624,-0.863923},{-0.498345,-0.866979},
{-0.493047,-0.870002},{-0.487731,-0.872994},{-0.482397,-0.875953},{-0.477045,-0.878879},{-0.471675,-0.881773},{-0.466287,-0.884633},{-0.460882,-0.887461},{-0.455460,-0.890256},
{-0.450021,-0.893018},{-0.444565,-0.895746},{-0.439093,-0.898442},{-0.433604,-0.901103},{-0.428100,-0.903732},{-0.422579,-0.906326},{-0.417043,-0.908887},{-0.411491,-0.911414},
{-0.405923,-0.913907},{-0.400341,-0.916366},{-0.394744,-0.918791},{-0.389132,-0.921182},{-0.383505,-0.923539},{-0.377865,-0.925861},{-0.372210,-0.928149},{-0.366541,-0.930402},
{-0.360859,-0.932620},{-0.355164,-0.934804},{-0.349455,-0.936953},{-0.343733,-0.939068},{-0.337998,-0.941147},{-0.332251,-0.943191},{-0.326491,-0.945200},{-0.320719,-0.947174},
{-0.314936,-0.949113},{-0.309140,-0.951016},{-0.303333,-0.952884},{-0.297515,-0.954717},{-0.291686,-0.956514},{-0.285846,-0.958276},{-0.279995,-0.960001},{-0.274134,-0.961692},
{-0.268263,-0.963346},{-0.262381,-0.964964},{-0.256490,-0.966547},{-0.250589,-0.968093},{-0.244679,-0.969604},{-0.238760,-0.971079},{-0.232832,-0.972517},{-0.226896,-0.973919},
{-0.220951,-0.975285},{-0.214997,-0.976615},{-0.209036,-0.977908},{-0.203067,-0.979165},{-0.197090,-0.980385},{-0.191106,-0.981569},{-0.185115,-0.982717},{-0.179117,-0.983828},
{-0.173112,-0.984902},{-0.167101,-0.985940},{-0.161084,-0.986941},{-0.155061,-0.987905},{-0.149032,-0.988832},{-0.142997,-0.989723},{-0.136957,-0.990577},{-0.130912,-0.991394},
{-0.124862,-0.992174},{-0.118808,-0.992917},{-0.112749,-0.993624},{-0.106686,-0.994293},{-0.100618,-0.994925},{-0.094548,-0.995520},{-0.088473,-0.996079},{-0.082395,-0.996600},
{-0.076315,-0.997084},{-0.070231,-0.997531},{-0.064145,-0.997941},{-0.058056,-0.998313},{-0.051966,-0.998649},{-0.045873,-0.998947},{-0.039778,-0.999209},{-0.033683,-0.999433},
{-0.027585,-0.999619},{-0.021487,-0.999769},{-0.015388,-0.999882},{-0.009289,-0.999957},{-0.003189,-0.999995},{0.002911,-0.999996},{0.009011,-0.999959},{0.015110,-0.999886},
{0.021209,-0.999775},{0.027308,-0.999627},{0.033405,-0.999442},{0.039501,-0.999220},{0.045595,-0.998960},{0.051688,-0.998663},{0.057779,-0.998329},{0.063868,-0.997958},
{0.069954,-0.997550},{0.076038,-0.997105},{0.082118,-0.996623},{0.088196,-0.996103},{0.094271,-0.995547},{0.100342,-0.994953},{0.106409,-0.994322},{0.112473,-0.993655},
{0.118532,-0.992950},{0.124586,-0.992209},{0.130637,-0.991430},{0.136682,-0.990615},{0.142722,-0.989763},{0.148757,-0.988874},{0.154786,-0.987948},{0.160810,-0.986985},
{0.166827,-0.985986},{0.172839,-0.984950},{0.178844,-0.983878},{0.184842,-0.982768},{0.190833,-0.981622},{0.196818,-0.980440},{0.202795,-0.979221},{0.208764,-0.977966},
{0.214726,-0.976674},{0.220679,-0.975346},{0.226625,-0.973982},{0.232562,-0.972582},{0.238490,-0.971145},{0.244410,-0.969672},{0.250320,-0.968163},{0.256221,-0.966618},
{0.262113,-0.965037},{0.267995,-0.963420},{0.273867,-0.961768},{0.279728,-0.960079},{0.285579,-0.958355},{0.291420,-0.956595},{0.297250,-0.954800},{0.303069,-0.952969},
{0.308876,-0.951102},{0.314672,-0.949200},{0.320456,-0.947263},{0.326228,-0.945291},{0.331989,-0.943283},{0.337736,-0.941241},{0.343472,-0.939163},{0.349194,-0.937050},
{0.354904,-0.934903},{0.360600,-0.932721},{0.366283,-0.930504},{0.371952,-0.928252},{0.377607,-0.925966},{0.383249,-0.923645},{0.388876,-0.921290},{0.394488,-0.918901},
{0.400086,-0.916477},{0.405669,-0.914020},{0.411237,-0.911528},{0.416790,-0.909003},{0.422327,-0.906444},{0.427848,-0.903850},{0.433354,-0.901224},{0.438843,-0.898564},
{0.444316,-0.895870},{0.449773,-0.893143},{0.455213,-0.890383},{0.460635,-0.887589},{0.466041,-0.884763},{0.471430,-0.881904},{0.476800,-0.879012},{0.482153,-0.876087},
{0.487489,-0.873129},{0.492806,-0.870139},{0.498104,-0.867117},{0.503384,-0.864063},{0.508646,-0.860976},{0.513888,-0.857857},{0.519111,-0.854707},{0.524315,-0.851524},
{0.529500,-0.848310},{0.534665,-0.845064},{0.539810,-0.841787},{0.544935,-0.838479},{0.550039,-0.835139},{0.555123,-0.831768},{0.560187,-0.828366},{0.565229,-0.824934},
{0.570251,-0.821471},{0.575251,-0.817977},{0.580230,-0.814453},{0.585187,-0.810898},{0.590123,-0.807313},{0.595036,-0.803699},{0.599928,-0.800054},{0.604797,-0.796380},
{0.609644,-0.792676},{0.614468,-0.788942},{0.619269,-0.785179},{0.624047,-0.781387},{0.628802,-0.777566},{0.633533,-0.773716},{0.638241,-0.769837},{0.642925,-0.765929},
{0.647585,-0.761993},{0.652221,-0.758029},{0.656833,-0.754036},{0.661420,-0.750015},{0.665983,-0.745967},{0.670521,-0.741890},{0.675034,-0.737786},{0.679522,-0.733655},
{0.683985,-0.729496},{0.688422,-0.725310},{0.692833,-0.721098},{0.697219,-0.716858},{0.701579,-0.712592},{0.705913,-0.708299},{0.710220,-0.703980},{0.714501,-0.699634},
{0.718756,-0.695263},{0.722983,-0.690865},{0.727184,-0.686442},{0.731358,-0.681994},{0.735505,-0.677520},{0.739624,-0.673021},{0.743715,-0.668496},{0.747779,-0.663947},
{0.751815,-0.659374},{0.755824,-0.654775},{0.759804,-0.650153},{0.763755,-0.645506},{0.767679,-0.640835},{0.771574,-0.636140},{0.775440,-0.631422},{0.779277,-0.626680},
{0.783085,-0.621915},{0.786864,-0.617126},{0.790614,-0.612315},{0.794334,-0.607481},{0.798025,-0.602624},{0.801686,-0.597745},{0.805318,-0.592844},{0.808919,-0.587920},
{0.812490,-0.582975},{0.816031,-0.578008},{0.819542,-0.573019},{0.823022,-0.568010},{0.826472,-0.562979},{0.829890,-0.557927},{0.833278,-0.552854},{0.836635,-0.547761},
{0.839961,-0.542647},{0.843255,-0.537513},{0.846518,-0.532359},{0.849750,-0.527186},{0.852950,-0.521993},{0.856118,-0.516780},{0.859255,-0.511548},{0.862359,-0.506297},
{0.865432,-0.501027},{0.868472,-0.495739},{0.871480,-0.490432},{0.874455,-0.485107},{0.877398,-0.479764},{0.880308,-0.474403},{0.883185,-0.469024},{0.886030,-0.463628},
{0.888842,-0.458214},{0.891620,-0.452784},{0.894366,-0.447337},{0.897078,-0.441873},{0.899756,-0.436392},{0.902402,-0.430896},{0.905013,-0.425383},{0.907591,-0.419855},
{0.910136,-0.414311},{0.912646,-0.408751},{0.915122,-0.403176},{0.917565,-0.397587},{0.919973,-0.391982},{0.922347,-0.386363},{0.924686,-0.380730},{0.926992,-0.375082},
{0.929262,-0.369420},{0.931499,-0.363745},{0.933700,-0.358056},{0.935867,-0.352354},{0.937999,-0.346639},{0.940096,-0.340911},{0.942158,-0.335170},{0.944185,-0.329416},
{0.946177,-0.323651},{0.948133,-0.317873},{0.950055,-0.312084},{0.951941,-0.306282},{0.953791,-0.300470},{0.955606,-0.294646},{0.957386,-0.288812},{0.959130,-0.282966},
{0.960838,-0.277110},{0.962511,-0.271244},{0.964147,-0.265368},{0.965748,-0.259482},{0.967313,-0.253586},{0.968842,-0.247680},{0.970335,-0.241766},{0.971791,-0.235842},
{0.973212,-0.229910},{0.974596,-0.223969},{0.975944,-0.218020},{0.977256,-0.212063},{0.978531,-0.206098},{0.979770,-0.200125},{0.980973,-0.194145},{0.982139,-0.188157},
{0.983268,-0.182163},{0.984361,-0.176161},{0.985418,-0.170153},{0.986437,-0.164139},{0.987420,-0.158119},{0.988366,-0.152093},{0.989276,-0.146061},{0.990148,-0.140024},
{0.990984,-0.133981},{0.991783,-0.127934},{0.992545,-0.121882},{0.993270,-0.115825},{0.993958,-0.109764},{0.994609,-0.103699},{0.995223,-0.097630},{0.995800,-0.091557},
{0.996340,-0.085481},{0.996843,-0.079402},{0.997309,-0.073319},{0.997737,-0.067235},{0.998129,-0.061147},{0.998483,-0.055057},{0.998800,-0.048966},{0.999081,-0.042872}
};

#endif