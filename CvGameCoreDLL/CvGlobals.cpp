//
// globals.cpp
//
#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvGamePlay.h"
#include "CvMap.h"
#include "CvInfos.h"
#include "CvPlayerAI.h"
#include "CvInfoWater.h"
#include "CvGameTextMgr.h"
#include "FVariableSystem.h"
#include "CvDLLUtilityIFaceBase.h"
#include "CvDLLXMLIFaceBase.h"

#define COPY(dst, src, typeName) \
	{ \
		int iNum = sizeof(src)/sizeof(typeName); \
		dst = new typeName[iNum]; \
		for (int i =0;i<iNum;i++) \
			dst[i] = src[i]; \
	}

template <class T>
void deleteInfoArray(std::vector<T*>& array)
{
	for (std::vector<T*>::iterator it = array.begin(); it != array.end(); ++it)
	{
		SAFE_DELETE(*it);
	}

	array.clear();
}

template <class T>
bool readInfoArray(FDataStreamBase* pStream, std::vector<T*>& array, const char* szClassName)
{
#if SERIALIZE_CVINFOS
	addToInfosVectors(&array);

	int iSize;
	pStream->Read(&iSize);
	FAssertMsg(iSize==sizeof(T), CvString::format("class size doesn't match cache size - check info read/write functions:%s", szClassName).c_str());
	if (iSize!=sizeof(T))
		return false;
	pStream->Read(&iSize);

	deleteInfoArray(array);

	for (int i = 0; i < iSize; ++i)
	{
		array.push_back(new T);
	}

	int iIndex = 0;
	for (std::vector<T*>::iterator it = array.begin(); it != array.end(); ++it)
	{
		(*it)->read(pStream);
		setInfoTypeFromString((*it)->getType(), iIndex);
		++iIndex;
	}

	return true;
#else
	FAssert(false);
	return false;
#endif
}

template <class T>
bool writeInfoArray(FDataStreamBase* pStream,  std::vector<T*>& array)
{
#if SERIALIZE_CVINFOS
	int iSize = sizeof(T);
	pStream->Write(iSize);
	pStream->Write(array.size());
	for (std::vector<T*>::iterator it = array.begin(); it != array.end(); ++it)
	{
		(*it)->write(pStream);
	}
	return true;
#else
	FAssert(false);
	return false;
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

CvGlobals gGlobals;

//
// CONSTRUCTOR
//
CvGlobals::CvGlobals() :
m_bGraphicsInitialized(false),
m_bLogging(false),
m_bRandLogging(false),
m_bOverwriteLogs(false),
m_bSynchLogging(false),
m_bDLLProfiler(false),
m_pkMainMenu(NULL),
m_iNewPlayers(0),
m_bZoomOut(false),
m_bZoomIn(false),
m_bLoadGameFromFile(false),
m_pFMPMgr(NULL),
m_asyncRand(NULL),
m_interface(NULL),
m_game(NULL),
m_messageQueue(NULL),
m_hotJoinMsgQueue(NULL),
m_messageControl(NULL),
m_messageCodes(NULL),
m_dropMgr(NULL),
m_portal(NULL),
m_setupData(NULL),
// <kmodx> Missing initialization
m_iniInitCore(NULL),
m_loadedInitCore(NULL),
m_iUSE_GET_BUILDING_COST_MOD_CALLBACK(0),
// </kmodx>
m_initCore(NULL),
m_statsReporter(NULL),
m_map(NULL),
m_diplomacyScreen(NULL),
m_mpDiplomacyScreen(NULL),
m_pathFinder(NULL),
m_interfacePathFinder(NULL),
m_stepFinder(NULL),
m_routeFinder(NULL),
m_borderFinder(NULL),
m_areaFinder(NULL),
m_plotGroupFinder(NULL),
m_pDLL(NULL),
m_aiPlotDirectionX(NULL),
m_aiPlotDirectionY(NULL),
m_aiPlotCardinalDirectionX(NULL),
m_aiPlotCardinalDirectionY(NULL),
m_aiCityPlotX(NULL),
m_aiCityPlotY(NULL),
m_aiCityPlotPriority(NULL),
m_aeTurnLeftDirection(NULL),
m_aeTurnRightDirection(NULL),
//m_aGameOptionsInfo(NULL),
//m_aPlayerOptionsInfo(NULL),
m_Profiler(NULL),
m_VarSystem(NULL),
m_paHints(NULL),
m_paMainMenus(NULL),
m_aiGlobalDefinesCache(NULL), // advc.003t, advc.003c
m_bHoFScreenUp(false), // advc.106i
m_fCAMERA_MIN_YAW(0), m_fCAMERA_MAX_YAW(0), m_fCAMERA_FAR_CLIP_Z_HEIGHT(0),
m_fCAMERA_MAX_TRAVEL_DISTANCE(0), m_fCAMERA_START_DISTANCE(0),
m_fAIR_BOMB_HEIGHT(0), m_fPLOT_SIZE(0), m_fCAMERA_SPECIAL_PITCH(0),
m_fCAMERA_MAX_TURN_OFFSET(0), m_fCAMERA_MIN_DISTANCE(0),
m_fCAMERA_UPPER_PITCH(0), m_fCAMERA_LOWER_PITCH(0),
m_fFIELD_OF_VIEW(0), m_fSHADOW_SCALE(0),
m_fUNIT_MULTISELECT_DISTANCE(0),
m_iUSE_FINISH_TEXT_CALLBACK(0), m_iUSE_ON_UPDATE_CALLBACK(0),
m_iUSE_CANNOT_FOUND_CITY_CALLBACK(0), m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK(0),
m_iUSE_IS_PLAYER_RESEARCH_CALLBACK(0), m_iUSE_CAN_RESEARCH_CALLBACK(0),
m_iUSE_CANNOT_DO_CIVIC_CALLBACK(0), m_iUSE_CAN_DO_CIVIC_CALLBACK(0),
m_iUSE_CANNOT_CONSTRUCT_CALLBACK(0), m_iUSE_CAN_CONSTRUCT_CALLBACK(0),
m_iUSE_CAN_DECLARE_WAR_CALLBACK(0), m_iUSE_CANNOT_RESEARCH_CALLBACK(0),
m_iUSE_GET_UNIT_COST_MOD_CALLBACK(0), m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK(0),
m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK(0), m_iUSE_CAN_BUILD_CALLBACK(0),
m_iUSE_CANNOT_TRAIN_CALLBACK(0), m_iUSE_CAN_TRAIN_CALLBACK(0),
m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK(0), m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK(0),
m_iUSE_ON_UNIT_SET_XY_CALLBACK(0), m_iUSE_ON_UNIT_SELECTED_CALLBACK(0),
m_iUSE_ON_UNIT_CREATED_CALLBACK(0), m_iUSE_ON_UNIT_LOST_CALLBACK(0),
// K-Mod
m_bUSE_AI_UNIT_UPDATE_CALLBACK(false), m_bUSE_AI_DO_DIPLO_CALLBACK(false),
m_bUSE_AI_CHOOSE_PRODUCTION_CALLBACK(false), m_bUSE_AI_DO_WAR_CALLBACK(false),
m_bUSE_AI_CHOOSE_TECH_CALLBACK(false),

m_bUSE_DO_GROWTH_CALLBACK(false), m_bUSE_DO_CULTURE_CALLBACK(false),
m_bUSE_DO_PLOT_CULTURE_CALLBACK(false), m_bUSE_DO_PRODUCTION_CALLBACK(false),
m_bUSE_DO_RELIGION_CALLBACK(false), m_bUSE_DO_GREAT_PEOPLE_CALLBACK(false),
m_bUSE_DO_MELTDOWN_CALLBACK(false), m_bUSE_DO_PILLAGE_GOLD_CALLBACK(false),
m_bUSE_GET_EXPERIENCE_NEEDED_CALLBACK(false), m_bUSE_UNIT_UPGRADE_PRICE_CALLBACK(false),
m_bUSE_DO_COMBAT_CALLBACK(false), // K-Mod end
// <advc.003b>
m_iRUINS_IMPROVEMENT(NO_IMPROVEMENT),
m_iDEFAULT_SPECIALIST(NO_SPECIALIST)
{
	m_aiWATER_TERRAIN[0] = m_aiWATER_TERRAIN[1] = -1; // </advc.003b>
}

CvGlobals::~CvGlobals()
{}

//
// allocate
//
void CvGlobals::init()
{
	//
	// These vars are used to initialize the globals.
	//

	int aiPlotDirectionX[NUM_DIRECTION_TYPES] =
	{
		0,	// DIRECTION_NORTH
		1,	// DIRECTION_NORTHEAST
		1,	// DIRECTION_EAST
		1,	// DIRECTION_SOUTHEAST
		0,	// DIRECTION_SOUTH
		-1,	// DIRECTION_SOUTHWEST
		-1,	// DIRECTION_WEST
		-1,	// DIRECTION_NORTHWEST
	};

	int aiPlotDirectionY[NUM_DIRECTION_TYPES] =
	{
		1,	// DIRECTION_NORTH
		1,	// DIRECTION_NORTHEAST
		0,	// DIRECTION_EAST
		-1,	// DIRECTION_SOUTHEAST
		-1,	// DIRECTION_SOUTH
		-1,	// DIRECTION_SOUTHWEST
		0,	// DIRECTION_WEST
		1,	// DIRECTION_NORTHWEST
	};

	int aiPlotCardinalDirectionX[NUM_CARDINALDIRECTION_TYPES] =
	{
		0,	// CARDINALDIRECTION_NORTH
		1,	// CARDINALDIRECTION_EAST
		0,	// CARDINALDIRECTION_SOUTH
		-1,	// CARDINALDIRECTION_WEST
	};

	int aiPlotCardinalDirectionY[NUM_CARDINALDIRECTION_TYPES] =
	{
		1,	// CARDINALDIRECTION_NORTH
		0,	// CARDINALDIRECTION_EAST
		-1,	// CARDINALDIRECTION_SOUTH
		0,	// CARDINALDIRECTION_WEST
	};

	int aiCityPlotX[NUM_CITY_PLOTS] =
	{
		0,
		0, 1, 1, 1, 0,-1,-1,-1,
		0, 1, 2, 2, 2, 1, 0,-1,-2,-2,-2,-1,
	};

	int aiCityPlotY[NUM_CITY_PLOTS] =
	{
		0,
		1, 1, 0,-1,-1,-1, 0, 1,
		2, 2, 1, 0,-1,-2,-2,-2,-1, 0, 1, 2,
	};

	int aiCityPlotPriority[NUM_CITY_PLOTS] =
	{
		0,
		1, 2, 1, 2, 1, 2, 1, 2,
		3, 4, 4, 3, 4, 4, 3, 4, 4, 3, 4, 4,
	};

	int aaiXYCityPlot[CITY_PLOTS_DIAMETER][CITY_PLOTS_DIAMETER] =
	{
		{-1, 17, 18, 19, -1,},

		{16, 6, 7, 8, 20,},

		{15, 5, 0, 1, 9,},

		{14, 4, 3, 2, 10,},

		{-1, 13, 12, 11, -1,}
	};

	DirectionTypes aeTurnRightDirection[NUM_DIRECTION_TYPES] =
	{
		DIRECTION_NORTHEAST,	// DIRECTION_NORTH
		DIRECTION_EAST,				// DIRECTION_NORTHEAST
		DIRECTION_SOUTHEAST,	// DIRECTION_EAST
		DIRECTION_SOUTH,			// DIRECTION_SOUTHEAST
		DIRECTION_SOUTHWEST,	// DIRECTION_SOUTH
		DIRECTION_WEST,				// DIRECTION_SOUTHWEST
		DIRECTION_NORTHWEST,	// DIRECTION_WEST
		DIRECTION_NORTH,			// DIRECTION_NORTHWEST
	};

	DirectionTypes aeTurnLeftDirection[NUM_DIRECTION_TYPES] =
	{
		DIRECTION_NORTHWEST,	// DIRECTION_NORTH
		DIRECTION_NORTH,			// DIRECTION_NORTHEAST
		DIRECTION_NORTHEAST,	// DIRECTION_EAST
		DIRECTION_EAST,				// DIRECTION_SOUTHEAST
		DIRECTION_SOUTHEAST,	// DIRECTION_SOUTH
		DIRECTION_SOUTH,			// DIRECTION_SOUTHWEST
		DIRECTION_SOUTHWEST,	// DIRECTION_WEST
		DIRECTION_WEST,				// DIRECTION_NORTHWEST
	};

	DirectionTypes aaeXYDirection[DIRECTION_DIAMETER][DIRECTION_DIAMETER] =
	{
		DIRECTION_SOUTHWEST, DIRECTION_WEST,	DIRECTION_NORTHWEST,
		DIRECTION_SOUTH,     NO_DIRECTION,    DIRECTION_NORTH,
		DIRECTION_SOUTHEAST, DIRECTION_EAST,	DIRECTION_NORTHEAST,
	};

	FAssertMsg(gDLL != NULL, "Civ app needs to set gDLL");

	m_VarSystem = new FVariableSystem();
	m_asyncRand = new CvRandom();
	m_initCore = new CvInitCore();
	m_loadedInitCore = new CvInitCore();
	m_iniInitCore = new CvInitCore();

	gDLL->initGlobals();	// some globals need to be allocated outside the dll

	m_game = new CvGameAI();
	m_map = new CvMap();

	CvPlayerAI::initStatics();
	CvTeamAI::initStatics();

	//m_pt3Origin = NiPoint3(0.0f, 0.0f, 0.0f); // advc.003j: unused

	COPY(m_aiPlotDirectionX, aiPlotDirectionX, int);
	COPY(m_aiPlotDirectionY, aiPlotDirectionY, int);
	COPY(m_aiPlotCardinalDirectionX, aiPlotCardinalDirectionX, int);
	COPY(m_aiPlotCardinalDirectionY, aiPlotCardinalDirectionY, int);
	COPY(m_aiCityPlotX, aiCityPlotX, int);
	COPY(m_aiCityPlotY, aiCityPlotY, int);
	COPY(m_aiCityPlotPriority, aiCityPlotPriority, int);
	COPY(m_aeTurnLeftDirection, aeTurnLeftDirection, DirectionTypes);
	COPY(m_aeTurnRightDirection, aeTurnRightDirection, DirectionTypes);
	memcpy(m_aaiXYCityPlot, aaiXYCityPlot, sizeof(m_aaiXYCityPlot));
	memcpy(m_aaeXYDirection, aaeXYDirection,sizeof(m_aaeXYDirection));
}

//
// free
//
void CvGlobals::uninit()
{
	//
	// See also CvXMLLoadUtilityInit.cpp::CleanUpGlobalVariables()
	//
	SAFE_DELETE_ARRAY(m_aiPlotDirectionX);
	SAFE_DELETE_ARRAY(m_aiPlotDirectionY);
	SAFE_DELETE_ARRAY(m_aiPlotCardinalDirectionX);
	SAFE_DELETE_ARRAY(m_aiPlotCardinalDirectionY);
	SAFE_DELETE_ARRAY(m_aiCityPlotX);
	SAFE_DELETE_ARRAY(m_aiCityPlotY);
	SAFE_DELETE_ARRAY(m_aiCityPlotPriority);
	SAFE_DELETE_ARRAY(m_aeTurnLeftDirection);
	SAFE_DELETE_ARRAY(m_aeTurnRightDirection);
	SAFE_DELETE_ARRAY(m_aiGlobalDefinesCache); // advc.003t

	SAFE_DELETE(m_game);
	SAFE_DELETE(m_map);

	CvPlayerAI::freeStatics();
	CvTeamAI::freeStatics();

	SAFE_DELETE(m_asyncRand);
	SAFE_DELETE(m_initCore);
	SAFE_DELETE(m_loadedInitCore);
	SAFE_DELETE(m_iniInitCore);
	gDLL->uninitGlobals();	// free globals allocated outside the dll
	SAFE_DELETE(m_VarSystem);

	// already deleted outside of the dll, set to null for safety
	m_messageQueue=NULL; m_hotJoinMsgQueue=NULL; m_messageControl=NULL;
	m_setupData=NULL; m_messageCodes=NULL; m_dropMgr=NULL;
	m_portal=NULL; m_statsReporter=NULL; m_interface=NULL;
	m_diplomacyScreen=NULL; m_mpDiplomacyScreen=NULL; m_pathFinder=NULL;
	m_interfacePathFinder=NULL; m_stepFinder=NULL; m_routeFinder=NULL;
	m_borderFinder=NULL; m_areaFinder=NULL; m_plotGroupFinder=NULL;

	m_typesMap.clear();
	m_aInfoVectors.clear();
}

void CvGlobals::clearTypesMap()
{
	m_typesMap.clear();
	if (m_VarSystem)
	{
		m_VarSystem->UnInit();
	}
}


CvDiplomacyScreen* CvGlobals::getDiplomacyScreen()
{
	return m_diplomacyScreen;
}

CMPDiplomacyScreen* CvGlobals::getMPDiplomacyScreen()
{
	return m_mpDiplomacyScreen;
}

CvMessageCodeTranslator& CvGlobals::getMessageCodes()
{
	return *m_messageCodes;
}

FMPIManager*& CvGlobals::getFMPMgrPtr()
{
	return m_pFMPMgr;
}

CvPortal& CvGlobals::getPortal()
{
	return *m_portal;
}

CvSetupData& CvGlobals::getSetupData()
{
	return *m_setupData;
}

CvInitCore& CvGlobals::getLoadedInitCore()
{
	return *m_loadedInitCore;
}

CvInitCore& CvGlobals::getIniInitCore()
{
	return *m_iniInitCore;
}

CvStatsReporter& CvGlobals::getStatsReporter()
{
	return *m_statsReporter;
}

CvStatsReporter* CvGlobals::getStatsReporterPtr()
{
	return m_statsReporter;
}

CvInterface& CvGlobals::getInterface()
{
	return *m_interface;
}

CvInterface* CvGlobals::getInterfacePtr()
{
	return m_interface;
}

CvRandom& CvGlobals::getASyncRand()
{
	return *m_asyncRand;
}

CMessageQueue& CvGlobals::getMessageQueue()
{
	return *m_messageQueue;
}

CMessageQueue& CvGlobals::getHotMessageQueue()
{
	return *m_hotJoinMsgQueue;
}

CMessageControl& CvGlobals::getMessageControl()
{
	return *m_messageControl;
}

CvDropMgr& CvGlobals::getDropMgr()
{
	return *m_dropMgr;
}

FAStar& CvGlobals::getPathFinder()
{
	return *m_pathFinder;
}

FAStar& CvGlobals::getInterfacePathFinder()
{
	return *m_interfacePathFinder;
}

FAStar& CvGlobals::getStepFinder()
{
	return *m_stepFinder;
}

FAStar& CvGlobals::getRouteFinder()
{
	return *m_routeFinder;
}

FAStar& CvGlobals::getBorderFinder()
{
	return *m_borderFinder;
}

FAStar& CvGlobals::getAreaFinder()
{
	return *m_areaFinder;
}

FAStar& CvGlobals::getPlotGroupFinder()
{
	return *m_plotGroupFinder;
}

// advc.003j: Was DLLExport, but actually unused.
/*NiPoint3& CvGlobals::getPt3Origin()
{
	return m_pt3Origin;
}*/

std::vector<CvInterfaceModeInfo*>& CvGlobals::getInterfaceModeInfo()
{
	return m_paInterfaceModeInfo;
}

CvInterfaceModeInfo& CvGlobals::getInterfaceModeInfo(InterfaceModeTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_INTERFACEMODE_TYPES);
	return *(m_paInterfaceModeInfo[e]);
}
// advc.003j: Was DLLExport, but actually neither called internally nor externally.
/*NiPoint3& CvGlobals::getPt3CameraDir()
{
	return m_pt3CameraDir;
}*/

bool& CvGlobals::getLogging()
{
	return m_bLogging;
}

bool& CvGlobals::getRandLogging()
{
	return m_bRandLogging;
}

bool& CvGlobals::getSynchLogging()
{
	return m_bSynchLogging;
}

bool& CvGlobals::overwriteLogs()
{
	return m_bOverwriteLogs;
}

int* CvGlobals::getPlotDirectionX()
{
	return m_aiPlotDirectionX;
}

int* CvGlobals::getPlotDirectionY()
{
	return m_aiPlotDirectionY;
}

int* CvGlobals::getPlotCardinalDirectionX()
{
	return m_aiPlotCardinalDirectionX;
}

int* CvGlobals::getPlotCardinalDirectionY()
{
	return m_aiPlotCardinalDirectionY;
}

int* CvGlobals::getCityPlotX()
{
	return m_aiCityPlotX;
}

int* CvGlobals::getCityPlotY()
{
	return m_aiCityPlotY;
}

int* CvGlobals::getCityPlotPriority()
{
	return m_aiCityPlotPriority;
}

int CvGlobals::getXYCityPlot(int i, int j)
{
	FAssertMsg(i < CITY_PLOTS_DIAMETER, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < CITY_PLOTS_DIAMETER, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_aaiXYCityPlot[i][j];
}

DirectionTypes* CvGlobals::getTurnLeftDirection()
{
	return m_aeTurnLeftDirection;
}

DirectionTypes CvGlobals::getTurnLeftDirection(int i)
{
	FAssertMsg(i < NUM_DIRECTION_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_aeTurnLeftDirection[i];
}

DirectionTypes* CvGlobals::getTurnRightDirection()
{
	return m_aeTurnRightDirection;
}

DirectionTypes CvGlobals::getTurnRightDirection(int i)
{
	FAssertMsg(i < NUM_DIRECTION_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_aeTurnRightDirection[i];
}

DirectionTypes CvGlobals::getXYDirection(int i, int j)
{
	FAssertMsg(i < DIRECTION_DIAMETER, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < DIRECTION_DIAMETER, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_aaeXYDirection[i][j];
}

CvColorInfo& CvGlobals::getColorInfo(ColorTypes e) const
{
	FAssert(e > -1);
	/*  <advc.106i> So that AdvCiv is able to show replays from mods with
		extra colors. And anyway, a bad color value shouldn't lead to a crash. */
	if(e >= getNumColorInfos()) {
		FAssert(m_bHoFScreenUp || e < getNumColorInfos());
		// +7: Skip colors from COLOR_CLEAR to COLOR_LIGHT_GREY
		e = (ColorTypes)((e + 7) % getNumColorInfos());
	} // </advc.106i>
	return *(m_paColorInfo[e]);
}

int CvGlobals::getActiveLandscapeID()
{
	return m_iActiveLandscapeID;
}

void CvGlobals::setActiveLandscapeID(int iLandscapeID)
{
	m_iActiveLandscapeID = iLandscapeID;
}

int& CvGlobals::getNumPlayableCivilizationInfos()
{
	return m_iNumPlayableCivilizationInfos;
}

int& CvGlobals::getNumAIPlayableCivilizationInfos()
{
	return m_iNumAIPlayableCivilizationInfos;
}

int& CvGlobals::getNumEntityEventTypes()
{
	return m_iNumEntityEventTypes;
}

CvString*& CvGlobals::getEntityEventTypes()
{
	return m_paszEntityEventTypes;
}

CvString& CvGlobals::getEntityEventTypes(EntityEventTypes e)
{
	FAssert(e > -1);
	FAssert(e < getNumEntityEventTypes());
	return m_paszEntityEventTypes[e];
}

int& CvGlobals::getNumAnimationOperatorTypes()
{
	return m_iNumAnimationOperatorTypes;
}

CvString*& CvGlobals::getAnimationOperatorTypes()
{
	return m_paszAnimationOperatorTypes;
}

CvString& CvGlobals::getAnimationOperatorTypes(AnimationOperatorTypes e)
{
	FAssert(e > -1);
	FAssert(e < getNumAnimationOperatorTypes());
	return m_paszAnimationOperatorTypes[e];
}

CvString*& CvGlobals::getFunctionTypes()
{
	return m_paszFunctionTypes;
}

CvString& CvGlobals::getFunctionTypes(FunctionTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_FUNC_TYPES);
	return m_paszFunctionTypes[e];
}

int& CvGlobals::getNumFlavorTypes()
{
	return m_iNumFlavorTypes;
}

CvString*& CvGlobals::getFlavorTypes()
{
	return m_paszFlavorTypes;
}

CvString& CvGlobals::getFlavorTypes(FlavorTypes e)
{
	FAssert(e > -1);
	FAssert(e < getNumFlavorTypes());
	return m_paszFlavorTypes[e];
}

int& CvGlobals::getNumArtStyleTypes()
{
	return m_iNumArtStyleTypes;
}

CvString*& CvGlobals::getArtStyleTypes()
{
	return m_paszArtStyleTypes;
}

CvString& CvGlobals::getArtStyleTypes(ArtStyleTypes e)
{
	FAssert(e > -1);
	FAssert(e < getNumArtStyleTypes());
	return m_paszArtStyleTypes[e];
}

int& CvGlobals::getNumCitySizeTypes()
{
	return m_iNumCitySizeTypes;
}

CvString*& CvGlobals::getCitySizeTypes()
{
	return m_paszCitySizeTypes;
}

CvString& CvGlobals::getCitySizeTypes(int i)
{
	FAssertMsg(i < getNumCitySizeTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszCitySizeTypes[i];
}

CvString*& CvGlobals::getContactTypes()
{
	return m_paszContactTypes;
}

CvString& CvGlobals::getContactTypes(ContactTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_CONTACT_TYPES);
	return m_paszContactTypes[e];
}

CvString*& CvGlobals::getDiplomacyPowerTypes()
{
	return m_paszDiplomacyPowerTypes;
}

CvString& CvGlobals::getDiplomacyPowerTypes(DiplomacyPowerTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_DIPLOMACYPOWER_TYPES);
	return m_paszDiplomacyPowerTypes[e];
}

CvString*& CvGlobals::getAutomateTypes()
{
	return m_paszAutomateTypes;
}

CvString& CvGlobals::getAutomateTypes(AutomateTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_AUTOMATE_TYPES);
	return m_paszAutomateTypes[e];
}

CvString*& CvGlobals::getDirectionTypes()
{
	return m_paszDirectionTypes;
}

CvString& CvGlobals::getDirectionTypes(AutomateTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_DIRECTION_TYPES);
	return m_paszDirectionTypes[e];
}

int& CvGlobals::getNumFootstepAudioTypes()
{
	return m_iNumFootstepAudioTypes;
}

CvString*& CvGlobals::getFootstepAudioTypes()
{
	return m_paszFootstepAudioTypes;
}

CvString& CvGlobals::getFootstepAudioTypes(int i)
{
	FAssertMsg(i < getNumFootstepAudioTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszFootstepAudioTypes[i];
}

int CvGlobals::getFootstepAudioTypeByTag(CvString strTag)
{
	int iIndex = -1;

	if (strTag.GetLength() <= 0)
	{
		return iIndex;
	}

	for (int i = 0; i < m_iNumFootstepAudioTypes; i++)
	{
		if (strTag.CompareNoCase(m_paszFootstepAudioTypes[i]) == 0)
		{
			iIndex = i;
			break;
		}
	}

	return iIndex;
}

CvString*& CvGlobals::getFootstepAudioTags()
{
	return m_paszFootstepAudioTags;
}

CvString& CvGlobals::getFootstepAudioTags(int i)
{
//	FAssertMsg(i < getNumFootstepAudioTags(), "Index out of bounds")
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszFootstepAudioTags[i];
}

void CvGlobals::setCurrentXMLFile(const TCHAR* szFileName)
{
	m_szCurrentXMLFile = szFileName;
}

CvString const& CvGlobals::getCurrentXMLFile() const
{
	return m_szCurrentXMLFile;
}
// advc.003t:
#define MAKE_STRINGS(VAR) #VAR,
// <advc.003t>
void CvGlobals::cacheGlobalInts(char const* szChangedDefine, int iNewValue)
{
	const char* const aszGlobalDefinesTagNames[] = {
		ENUMERATE_GLOBAL_DEFINES(MAKE_STRINGS)
	};
	FAssert(sizeof(aszGlobalDefinesTagNames) / sizeof(char*) == NUM_GLOBAL_DEFINES);

	if (szChangedDefine != NULL) // Cache update
	{
		for (int i = 0; i < NUM_GLOBAL_DEFINES; i++)
		{
			if (std::strcmp(aszGlobalDefinesTagNames[i], szChangedDefine) == 0)
			{
				m_aiGlobalDefinesCache[i] = iNewValue;
				break;
			}
		}
		return; // Don't bother checking the Python callback flags
	}

	// Initialize cache (or full update)
	SAFE_DELETE_ARRAY(m_aiGlobalDefinesCache);
	m_aiGlobalDefinesCache = new int[NUM_GLOBAL_DEFINES];
	for (int i = 0; i < NUM_GLOBAL_DEFINES; i++)
	{
		/*  Let's not throw away the default values from BBAI
			(though they should of course not be needed) */
		int iDefault = 0;
		switch((GlobalDefines)i)
		{
		// BETTER_BTS_AI_MOD, Efficiency, Options, 02/21/10, jdog5000: START
		// BBAI AI Variables
		case WAR_SUCCESS_CITY_CAPTURING: iDefault = 25; break;
		case BBAI_ATTACK_CITY_STACK_RATIO: iDefault = 110; break;
		case BBAI_SKIP_BOMBARD_BASE_STACK_RATIO: iDefault = 300; break;
		case BBAI_SKIP_BOMBARD_MIN_STACK_RATIO: iDefault = 140; break;
		//case TECH_COST_FIRST_KNOWN_PREREQ_MODIFIER: iDefault = 20; break; // advc.910: Should be 0 also by default
		case TECH_COST_FIRST_KNOWN_PREREQ_MODIFIER: iDefault = 20; break;
		case TECH_COST_KNOWN_PREREQ_MODIFIER: iDefault = 20; break;
		// From Lead From Behind by UncutDragon
		case LFB_ENABLE: iDefault = 1; break;
		case LFB_BASEDONGENERAL: iDefault = 1; break;
		case LFB_BASEDONEXPERIENCE: iDefault = 1; break;
		case LFB_BASEDONLIMITED: iDefault = 1; break;
		case LFB_BASEDONHEALER: iDefault = 1; break;
		case LFB_DEFENSIVEADJUSTMENT: iDefault = 1; break;
		case LFB_USESLIDINGSCALE: iDefault = 1; break;
		case LFB_ADJUSTNUMERATOR: iDefault = 1; break;
		case LFB_ADJUSTDENOMINATOR: iDefault = 3; break;
		case LFB_USECOMBATODDS: iDefault = 1; break;
		case COMBAT_DIE_SIDES: iDefault = -1; break;
		case COMBAT_DAMAGE: iDefault = -1; break;
		// BETTER_BTS_AI_MOD: END
		}
		m_aiGlobalDefinesCache[i] = getDefineINT(aszGlobalDefinesTagNames[i], iDefault);
	} // </advc.003t>

	m_iUSE_FINISH_TEXT_CALLBACK = getDefineINT("USE_FINISH_TEXT_CALLBACK");
	m_iUSE_CANNOT_FOUND_CITY_CALLBACK = getDefineINT("USE_CANNOT_FOUND_CITY_CALLBACK");
	m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK = getDefineINT("USE_CAN_FOUND_CITIES_ON_WATER_CALLBACK");
	m_iUSE_IS_PLAYER_RESEARCH_CALLBACK = getDefineINT("USE_IS_PLAYER_RESEARCH_CALLBACK");
	m_iUSE_CAN_RESEARCH_CALLBACK = getDefineINT("USE_CAN_RESEARCH_CALLBACK");
	m_iUSE_CANNOT_DO_CIVIC_CALLBACK = getDefineINT("USE_CANNOT_DO_CIVIC_CALLBACK");
	m_iUSE_CAN_DO_CIVIC_CALLBACK = getDefineINT("USE_CAN_DO_CIVIC_CALLBACK");
	m_iUSE_CANNOT_CONSTRUCT_CALLBACK = getDefineINT("USE_CANNOT_CONSTRUCT_CALLBACK");
	m_iUSE_CAN_CONSTRUCT_CALLBACK = getDefineINT("USE_CAN_CONSTRUCT_CALLBACK");
	m_iUSE_CAN_DECLARE_WAR_CALLBACK = getDefineINT("USE_CAN_DECLARE_WAR_CALLBACK");
	m_iUSE_CANNOT_RESEARCH_CALLBACK = getDefineINT("USE_CANNOT_RESEARCH_CALLBACK");
	m_iUSE_GET_UNIT_COST_MOD_CALLBACK = getDefineINT("USE_GET_UNIT_COST_MOD_CALLBACK");
	m_iUSE_GET_BUILDING_COST_MOD_CALLBACK = getDefineINT("USE_GET_BUILDING_COST_MOD_CALLBACK");
	m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK = getDefineINT("USE_GET_CITY_FOUND_VALUE_CALLBACK");
	m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK = getDefineINT("USE_CANNOT_HANDLE_ACTION_CALLBACK");
	m_iUSE_CAN_BUILD_CALLBACK = getDefineINT("USE_CAN_BUILD_CALLBACK");
	m_iUSE_CANNOT_TRAIN_CALLBACK = getDefineINT("USE_CANNOT_TRAIN_CALLBACK");
	m_iUSE_CAN_TRAIN_CALLBACK = getDefineINT("USE_CAN_TRAIN_CALLBACK");
	m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK = getDefineINT("USE_UNIT_CANNOT_MOVE_INTO_CALLBACK");
	/*  advc.003c, advc.001: Had said "USE_USE...".
		The variable name is still wrong, but that doesn't hurt. */
	m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK = getDefineINT("USE_CANNOT_SPREAD_RELIGION_CALLBACK");
	m_iUSE_ON_UNIT_SET_XY_CALLBACK = getDefineINT("USE_ON_UNIT_SET_XY_CALLBACK");
	m_iUSE_ON_UNIT_SELECTED_CALLBACK = getDefineINT("USE_ON_UNIT_SELECTED_CALLBACK");
	m_iUSE_ON_UPDATE_CALLBACK = getDefineINT("USE_ON_UPDATE_CALLBACK");
	m_iUSE_ON_UNIT_CREATED_CALLBACK = getDefineINT("USE_ON_UNIT_CREATED_CALLBACK");
	m_iUSE_ON_UNIT_LOST_CALLBACK = getDefineINT("USE_ON_UNIT_LOST_CALLBACK");
	// K-Mod
	m_bUSE_AI_UNIT_UPDATE_CALLBACK = getDefineINT("USE_AI_UNIT_UPDATE_CALLBACK") != 0;
	m_bUSE_AI_DO_DIPLO_CALLBACK = getDefineINT("USE_AI_DO_DIPLO_CALLBACK") != 0;
	m_bUSE_AI_CHOOSE_PRODUCTION_CALLBACK = getDefineINT("USE_AI_CHOOSE_PRODUCTION_CALLBACK") != 0;
	m_bUSE_AI_DO_WAR_CALLBACK = getDefineINT("USE_AI_DO_WAR_CALLBACK") != 0;
	m_bUSE_AI_CHOOSE_TECH_CALLBACK = getDefineINT("USE_AI_CHOOSE_TECH_CALLBACK") != 0;

	m_bUSE_DO_GROWTH_CALLBACK = getDefineINT("USE_DO_GROWTH_CALLBACK") != 0;
	m_bUSE_DO_CULTURE_CALLBACK = getDefineINT("USE_DO_CULTURE_CALLBACK") != 0;
	m_bUSE_DO_PLOT_CULTURE_CALLBACK = getDefineINT("USE_DO_PLOT_CULTURE_CALLBACK") != 0;
	m_bUSE_DO_PRODUCTION_CALLBACK = getDefineINT("USE_DO_PRODUCTION_CALLBACK") != 0;
	m_bUSE_DO_RELIGION_CALLBACK = getDefineINT("USE_DO_RELIGION_CALLBACK") != 0;
	m_bUSE_DO_GREAT_PEOPLE_CALLBACK = getDefineINT("USE_DO_GREAT_PEOPLE_CALLBACK") != 0;
	m_bUSE_DO_MELTDOWN_CALLBACK = getDefineINT("USE_DO_MELTDOWN_CALLBACK") != 0;

	m_bUSE_DO_PILLAGE_GOLD_CALLBACK = getDefineINT("USE_DO_PILLAGE_GOLD_CALLBACK") != 0;
	m_bUSE_GET_EXPERIENCE_NEEDED_CALLBACK = getDefineINT("USE_GET_EXPERIENCE_NEEDED_CALLBACK") != 0;
	m_bUSE_UNIT_UPGRADE_PRICE_CALLBACK = getDefineINT("USE_UNIT_UPGRADE_PRICE_CALLBACK") != 0;
	m_bUSE_DO_COMBAT_CALLBACK = getDefineINT("USE_DO_COMBAT_CALLBACK") != 0;
	// K-Mod end
}

void CvGlobals::cacheGlobalFloats()
{
	m_fPOWER_CORRECTION = getDefineFLOAT("POWER_CORRECTION"); // advc.104

	m_fCAMERA_MIN_YAW = getDefineFLOAT("CAMERA_MIN_YAW");
	m_fCAMERA_MAX_YAW = getDefineFLOAT("CAMERA_MAX_YAW");
	m_fCAMERA_FAR_CLIP_Z_HEIGHT = getDefineFLOAT("CAMERA_FAR_CLIP_Z_HEIGHT");
	m_fCAMERA_MAX_TRAVEL_DISTANCE = getDefineFLOAT("CAMERA_MAX_TRAVEL_DISTANCE");
	m_fCAMERA_START_DISTANCE = getDefineFLOAT("CAMERA_START_DISTANCE");
	m_fAIR_BOMB_HEIGHT = getDefineFLOAT("AIR_BOMB_HEIGHT");
	m_fPLOT_SIZE = getDefineFLOAT("PLOT_SIZE");
	m_fCAMERA_SPECIAL_PITCH = getDefineFLOAT("CAMERA_SPECIAL_PITCH");
	m_fCAMERA_MAX_TURN_OFFSET = getDefineFLOAT("CAMERA_MAX_TURN_OFFSET");
	m_fCAMERA_MIN_DISTANCE = getDefineFLOAT("CAMERA_MIN_DISTANCE");
	m_fCAMERA_UPPER_PITCH = getDefineFLOAT("CAMERA_UPPER_PITCH");
	m_fCAMERA_LOWER_PITCH = getDefineFLOAT("CAMERA_LOWER_PITCH");
	m_fFIELD_OF_VIEW = getDefineFLOAT("FIELD_OF_VIEW");
	m_fSHADOW_SCALE = getDefineFLOAT("SHADOW_SCALE");
	m_fUNIT_MULTISELECT_DISTANCE = getDefineFLOAT("UNIT_MULTISELECT_DISTANCE");
}

void CvGlobals::cacheGlobals()
{
	// <advc.003t> Moved into subroutines to allow partial updates
	cacheGlobalInts();
	cacheGlobalFloats();
	// Strings: Mostly can't cache these here (too early) // </advc.003t>
}

// <advc.003b>
void CvGlobals::setRUINS_IMPROVEMENT(int iValue) {

	m_iRUINS_IMPROVEMENT = iValue;
}

void CvGlobals::setWATER_TERRAIN(bool bShallow, int iValue) {

	m_aiWATER_TERRAIN[bShallow] = iValue;
} 

void CvGlobals::setDEFAULT_SPECIALIST(int iValue) {

	m_iDEFAULT_SPECIALIST = iValue;
} // </advc.003b>

int CvGlobals::getDefineINT(const char * szName,
		// BETTER_BTS_AI_MOD, 02/21/10, jdog5000: START
		const int iDefault) const
{
	int iReturn = iDefault;
	// BETTER_BTS_AI_MOD: END
	bool bSuccess = // advc.003c
			getDefinesVarSystem()->GetValue(szName, iReturn);
	FAssert(bSuccess); // advc.003c
	return iReturn;
}


float CvGlobals::getDefineFLOAT(const char * szName) const
{
	float fReturn = 0;
	bool bSuccess = // advc.003c
			getDefinesVarSystem()->GetValue(szName, fReturn);
	/*  advc.003c: The EXE queries CAMERA_MIN_DISTANCE during startup, which
		fails but doesn't cause any problems. */
	FAssert(bSuccess || std::strcmp("CAMERA_MIN_DISTANCE", szName) == 0);
	return fReturn;
}

const char * CvGlobals::getDefineSTRING(const char * szName) const
{
	const char * szReturn = NULL;
	bool bSuccess = // advc.003c
			getDefinesVarSystem()->GetValue(szName, szReturn);
	FAssert(bSuccess); // advc.003c
	return szReturn;
}

void CvGlobals::setDefineINT(const char * szName, int iValue,
		bool bUpdateCache) // advc.003t
{
	getDefinesVarSystem()->SetValue(szName, iValue);
	// <advc.003t>
	if(bUpdateCache)
		cacheGlobalInts(szName, iValue); // Pinpoint update </advc.003t>
}

void CvGlobals::setDefineFLOAT(const char * szName, float fValue,
		bool bUpdateCache) // advc.003t
{
	getDefinesVarSystem()->SetValue(szName, fValue);
	// <advc.003t>
	if(bUpdateCache)
		cacheGlobalFloats(); // </advc.003t>
}

void CvGlobals::setDefineSTRING(const char * szName, const char * szValue,
		bool bUpdateCache) // advc.003b
{
	getDefinesVarSystem()->SetValue(szName, szValue);
	//cacheGlobals();
	FAssertMsg(!bUpdateCache, "No strings to update"); // advc.003t
}
/*  <advc.003t> Optional parameters added. The return value is only an upper bound,
	even if an argument is given. */
int CvGlobals::getNUM_UNIT_PREREQ_OR_BONUSES(UnitTypes eUnit) const
{
	return (eUnit == NO_UNIT || getUnitInfo(eUnit).isAnyPrereqOrBonus() ?
			getDefineINT(NUM_UNIT_PREREQ_OR_BONUSES) : 0);
}

int CvGlobals::getNUM_UNIT_AND_TECH_PREREQS(UnitTypes eUnit) const
{
	return (eUnit == NO_UNIT || getUnitInfo(eUnit).isAnyPrereqAndTech() ?
			getDefineINT(NUM_UNIT_AND_TECH_PREREQS) : 0);
}

int CvGlobals::getNUM_BUILDING_PREREQ_OR_BONUSES(BuildingTypes eBuilding) const
{
	return (eBuilding == NO_BUILDING || getBuildingInfo(eBuilding).isAnyPrereqOrBonus() ?
			getDefineINT(NUM_BUILDING_PREREQ_OR_BONUSES) : 0);
}

int CvGlobals::getNUM_BUILDING_AND_TECH_PREREQS(BuildingTypes eBuilding) const
{
	return (eBuilding == NO_BUILDING || getBuildingInfo(eBuilding).isAnyPrereqAndTech() ?
			getDefineINT(NUM_BUILDING_AND_TECH_PREREQS) : 0);
}

int CvGlobals::getNUM_AND_TECH_PREREQS(TechTypes eTech) const
{
	return (eTech == NO_TECH || getTechInfo(eTech).isAnyPrereqAndTech() ?
			getDefineINT(NUM_AND_TECH_PREREQS) : 0);
}

int CvGlobals::getNUM_OR_TECH_PREREQS(TechTypes eTech) const
{
	return (eTech == NO_TECH || getTechInfo(eTech).isAnyPrereqOrTech() ?
			getDefineINT(NUM_OR_TECH_PREREQS) : 0);
}

int CvGlobals::getNUM_ROUTE_PREREQ_OR_BONUSES(RouteTypes eRoute) const
{
	return (eRoute == NO_ROUTE || getRouteInfo(eRoute).isAnyPrereqOrBonus() ?
			getDefineINT(NUM_ROUTE_PREREQ_OR_BONUSES) : 0);
}

int CvGlobals::getNUM_CORPORATION_PREREQ_BONUSES(CorporationTypes eCorporation) const
{
	return (eCorporation == NO_CORPORATION || getCorporationInfo(eCorporation).isAnyPrereqOrBonus() ?
			getDefineINT(NUM_CORPORATION_PREREQ_BONUSES) : 0);
}
// </advc.003t>

float CvGlobals::getCAMERA_MIN_YAW()
{
	return m_fCAMERA_MIN_YAW;
}

float CvGlobals::getCAMERA_MAX_YAW()
{
	return m_fCAMERA_MAX_YAW;
}

float CvGlobals::getCAMERA_FAR_CLIP_Z_HEIGHT()
{
	return m_fCAMERA_FAR_CLIP_Z_HEIGHT;
}

float CvGlobals::getCAMERA_MAX_TRAVEL_DISTANCE()
{
	return m_fCAMERA_MAX_TRAVEL_DISTANCE;
}

float CvGlobals::getCAMERA_START_DISTANCE()
{
	return m_fCAMERA_START_DISTANCE;
}

float CvGlobals::getAIR_BOMB_HEIGHT()
{
	return m_fAIR_BOMB_HEIGHT;
}

float CvGlobals::getPLOT_SIZE()
{
	return m_fPLOT_SIZE;
}

float CvGlobals::getCAMERA_SPECIAL_PITCH()
{
	return m_fCAMERA_SPECIAL_PITCH;
}

float CvGlobals::getCAMERA_MAX_TURN_OFFSET()
{
	return m_fCAMERA_MAX_TURN_OFFSET;
}

float CvGlobals::getCAMERA_MIN_DISTANCE()
{
	return m_fCAMERA_MIN_DISTANCE;
}

float CvGlobals::getCAMERA_UPPER_PITCH()
{
	return m_fCAMERA_UPPER_PITCH;
}

float CvGlobals::getCAMERA_LOWER_PITCH()
{
	return m_fCAMERA_LOWER_PITCH;
}

float CvGlobals::getFIELD_OF_VIEW()
{
	return m_fFIELD_OF_VIEW;
}

float CvGlobals::getSHADOW_SCALE()
{
	return m_fSHADOW_SCALE;
}

float CvGlobals::getUNIT_MULTISELECT_DISTANCE()
{
	return m_fUNIT_MULTISELECT_DISTANCE;
}

int CvGlobals::getUSE_CANNOT_FOUND_CITY_CALLBACK()
{
	return m_iUSE_CANNOT_FOUND_CITY_CALLBACK;
}

int CvGlobals::getUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK()
{
	return m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK;
}

int CvGlobals::getUSE_IS_PLAYER_RESEARCH_CALLBACK()
{
	return m_iUSE_IS_PLAYER_RESEARCH_CALLBACK;
}

int CvGlobals::getUSE_CAN_RESEARCH_CALLBACK()
{
	return m_iUSE_CAN_RESEARCH_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_DO_CIVIC_CALLBACK()
{
	return m_iUSE_CANNOT_DO_CIVIC_CALLBACK;
}

int CvGlobals::getUSE_CAN_DO_CIVIC_CALLBACK()
{
	return m_iUSE_CAN_DO_CIVIC_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_CONSTRUCT_CALLBACK()
{
	return m_iUSE_CANNOT_CONSTRUCT_CALLBACK;
}

int CvGlobals::getUSE_CAN_CONSTRUCT_CALLBACK()
{
	return m_iUSE_CAN_CONSTRUCT_CALLBACK;
}

int CvGlobals::getUSE_CAN_DECLARE_WAR_CALLBACK()
{
	return m_iUSE_CAN_DECLARE_WAR_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_RESEARCH_CALLBACK()
{
	return m_iUSE_CANNOT_RESEARCH_CALLBACK;
}

int CvGlobals::getUSE_GET_UNIT_COST_MOD_CALLBACK()
{
	return m_iUSE_GET_UNIT_COST_MOD_CALLBACK;
}

int CvGlobals::getUSE_GET_BUILDING_COST_MOD_CALLBACK()
{
	return m_iUSE_GET_BUILDING_COST_MOD_CALLBACK;
}

int CvGlobals::getUSE_GET_CITY_FOUND_VALUE_CALLBACK()
{
	return m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_HANDLE_ACTION_CALLBACK()
{
	return m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK;
}

int CvGlobals::getUSE_CAN_BUILD_CALLBACK()
{
	return m_iUSE_CAN_BUILD_CALLBACK;
}

int CvGlobals::getUSE_CANNOT_TRAIN_CALLBACK()
{
	return m_iUSE_CANNOT_TRAIN_CALLBACK;
}

int CvGlobals::getUSE_CAN_TRAIN_CALLBACK()
{
	return m_iUSE_CAN_TRAIN_CALLBACK;
}

int CvGlobals::getUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK()
{
	return m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK;
}

int CvGlobals::getUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK()
{
	return m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK;
}

int CvGlobals::getUSE_FINISH_TEXT_CALLBACK()
{
	return m_iUSE_FINISH_TEXT_CALLBACK;
}

int CvGlobals::getUSE_ON_UNIT_SET_XY_CALLBACK()
{
	return m_iUSE_ON_UNIT_SET_XY_CALLBACK;
}

int CvGlobals::getUSE_ON_UNIT_SELECTED_CALLBACK()
{
	return m_iUSE_ON_UNIT_SELECTED_CALLBACK;
}

int CvGlobals::getUSE_ON_UPDATE_CALLBACK()
{
	return m_iUSE_ON_UPDATE_CALLBACK;
}

int CvGlobals::getUSE_ON_UNIT_CREATED_CALLBACK()
{
	return m_iUSE_ON_UNIT_CREATED_CALLBACK;
}

int CvGlobals::getUSE_ON_UNIT_LOST_CALLBACK()
{
	return m_iUSE_ON_UNIT_LOST_CALLBACK;
}

int CvGlobals::getMAX_CIV_PLAYERS()
{
	return MAX_CIV_PLAYERS;
}

int CvGlobals::getMAX_PLAYERS()
{
	return MAX_PLAYERS;
}

int CvGlobals::getMAX_CIV_TEAMS()
{
	return MAX_CIV_TEAMS;
}

int CvGlobals::getMAX_TEAMS()
{
	return MAX_TEAMS;
}

int CvGlobals::getBARBARIAN_PLAYER()
{
	return BARBARIAN_PLAYER;
}

int CvGlobals::getBARBARIAN_TEAM()
{
	return BARBARIAN_TEAM;
}

int CvGlobals::getINVALID_PLOT_COORD()
{
	return INVALID_PLOT_COORD;
}

int CvGlobals::getNUM_CITY_PLOTS()
{
	return NUM_CITY_PLOTS;
}

int CvGlobals::getCITY_HOME_PLOT()
{
	return CITY_HOME_PLOT;
}

void CvGlobals::setDLLIFace(CvDLLUtilityIFaceBase* pDll)
{
	m_pDLL = pDll;
}

void CvGlobals::setDLLProfiler(FProfiler* prof)
{
	m_Profiler=prof;
}

FProfiler* CvGlobals::getDLLProfiler()
{
	return m_Profiler;
}

void CvGlobals::enableDLLProfiler(bool bEnable)
{
	m_bDLLProfiler = bEnable;
}

bool CvGlobals::isDLLProfilerEnabled() const
{
	//return m_bDLLProfiler;
	// K-Mod. (I don't know how to enable this in-game...)
#ifdef FP_PROFILE_ENABLE
	return true;
#else
	return false;
#endif
	// K-Mod end
}

bool CvGlobals::readBuildingInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paBuildingInfo, "CvBuildingInfo");
}

void CvGlobals::writeBuildingInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paBuildingInfo);
}

bool CvGlobals::readTechInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paTechInfo, "CvTechInfo");
}

void CvGlobals::writeTechInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paTechInfo);
}

bool CvGlobals::readUnitInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paUnitInfo, "CvUnitInfo");
}

void CvGlobals::writeUnitInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paUnitInfo);
}

bool CvGlobals::readLeaderHeadInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paLeaderHeadInfo, "CvLeaderHeadInfo");
}

void CvGlobals::writeLeaderHeadInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paLeaderHeadInfo);
}

bool CvGlobals::readCivilizationInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paCivilizationInfo, "CvCivilizationInfo");
}

void CvGlobals::writeCivilizationInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paCivilizationInfo);
}

bool CvGlobals::readPromotionInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paPromotionInfo, "CvPromotionInfo");
}

void CvGlobals::writePromotionInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paPromotionInfo);
}

bool CvGlobals::readDiplomacyInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paDiplomacyInfo, "CvDiplomacyInfo");
}

void CvGlobals::writeDiplomacyInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paDiplomacyInfo);
}

bool CvGlobals::readCivicInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paCivicInfo, "CvCivicInfo");
}

void CvGlobals::writeCivicInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paCivicInfo);
}

bool CvGlobals::readHandicapInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paHandicapInfo, "CvHandicapInfo");
}

void CvGlobals::writeHandicapInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paHandicapInfo);
}

bool CvGlobals::readBonusInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paBonusInfo, "CvBonusInfo");
}

void CvGlobals::writeBonusInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paBonusInfo);
}

bool CvGlobals::readImprovementInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paImprovementInfo, "CvImprovementInfo");
}

void CvGlobals::writeImprovementInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paImprovementInfo);
}

bool CvGlobals::readEventInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paEventInfo, "CvEventInfo");
}

void CvGlobals::writeEventInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paEventInfo);
}

bool CvGlobals::readEventTriggerInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paEventTriggerInfo, "CvEventTriggerInfo");
}

void CvGlobals::writeEventTriggerInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paEventTriggerInfo);
}


//
// Global Types Hash Map
//

int CvGlobals::getTypesEnum(const char* szType) const
{
	FAssertMsg(szType, "null type string");
	TypesMap::const_iterator it = m_typesMap.find(szType);
	if (it!=m_typesMap.end())
	{
		return it->second;
	}

	FAssertMsg(strcmp(szType, "NONE")==0 || strcmp(szType, "")==0, CvString::format("type %s not found", szType).c_str());
	return -1;
}

void CvGlobals::setTypesEnum(const char* szType, int iEnum)
{
	FAssertMsg(szType, "null type string");
	FAssertMsg(m_typesMap.find(szType)==m_typesMap.end(), "types entry already exists");
	m_typesMap[szType] = iEnum;
}


int CvGlobals::getNUM_ENGINE_DIRTY_BITS() const
{
	return NUM_ENGINE_DIRTY_BITS;
}

int CvGlobals::getNUM_INTERFACE_DIRTY_BITS() const
{
	return NUM_INTERFACE_DIRTY_BITS;
}

int CvGlobals::getNUM_YIELD_TYPES() const
{
	return NUM_YIELD_TYPES;
}

int CvGlobals::getNUM_COMMERCE_TYPES() const
{
	return NUM_COMMERCE_TYPES;
}

int CvGlobals::getNUM_FORCECONTROL_TYPES() const
{
	return NUM_FORCECONTROL_TYPES;
}

int CvGlobals::getNUM_INFOBAR_TYPES() const
{
	return NUM_INFOBAR_TYPES;
}

int CvGlobals::getNUM_HEALTHBAR_TYPES() const
{
	return NUM_HEALTHBAR_TYPES;
}

int CvGlobals::getNUM_CONTROL_TYPES() const
{
	return NUM_CONTROL_TYPES;
}

int CvGlobals::getNUM_LEADERANIM_TYPES() const
{
	return NUM_LEADERANIM_TYPES;
}


void CvGlobals::deleteInfoArrays()
{
	deleteInfoArray(m_paBuildingClassInfo);
	deleteInfoArray(m_paBuildingInfo);
	deleteInfoArray(m_paSpecialBuildingInfo);

	deleteInfoArray(m_paLeaderHeadInfo);
	deleteInfoArray(m_paTraitInfo);
	deleteInfoArray(m_paCivilizationInfo);
	deleteInfoArray(m_paUnitArtStyleTypeInfo);

	deleteInfoArray(m_paVoteSourceInfo);
	deleteInfoArray(m_paHints);
	deleteInfoArray(m_paMainMenus);
	deleteInfoArray(m_paGoodyInfo);
	deleteInfoArray(m_paHandicapInfo);
	deleteInfoArray(m_paGameSpeedInfo);
	deleteInfoArray(m_paTurnTimerInfo);
	deleteInfoArray(m_paVictoryInfo);
	deleteInfoArray(m_paHurryInfo);
	deleteInfoArray(m_paWorldInfo);
	deleteInfoArray(m_paSeaLevelInfo);
	deleteInfoArray(m_paClimateInfo);
	deleteInfoArray(m_paProcessInfo);
	deleteInfoArray(m_paVoteInfo);
	deleteInfoArray(m_paProjectInfo);
	deleteInfoArray(m_paReligionInfo);
	deleteInfoArray(m_paCorporationInfo);
	deleteInfoArray(m_paCommerceInfo);
	deleteInfoArray(m_paEmphasizeInfo);
	deleteInfoArray(m_paUpkeepInfo);
	deleteInfoArray(m_paCultureLevelInfo);

	deleteInfoArray(m_paColorInfo);
	deleteInfoArray(m_paPlayerColorInfo);
	deleteInfoArray(m_paInterfaceModeInfo);
	deleteInfoArray(m_paCameraInfo);
	deleteInfoArray(m_paAdvisorInfo);
	deleteInfoArray(m_paThroneRoomCamera);
	deleteInfoArray(m_paThroneRoomInfo);
	deleteInfoArray(m_paThroneRoomStyleInfo);
	deleteInfoArray(m_paSlideShowInfo);
	deleteInfoArray(m_paSlideShowRandomInfo);
	deleteInfoArray(m_paWorldPickerInfo);
	deleteInfoArray(m_paSpaceShipInfo);

	deleteInfoArray(m_paCivicInfo);
	deleteInfoArray(m_paImprovementInfo);

	deleteInfoArray(m_paRouteInfo);
	deleteInfoArray(m_paRouteModelInfo);
	deleteInfoArray(m_paRiverInfo);
	deleteInfoArray(m_paRiverModelInfo);

	deleteInfoArray(m_paWaterPlaneInfo);
	deleteInfoArray(m_paTerrainPlaneInfo);
	deleteInfoArray(m_paCameraOverlayInfo);

	deleteInfoArray(m_aEraInfo);
	deleteInfoArray(m_paEffectInfo);
	deleteInfoArray(m_paAttachableInfo);

	deleteInfoArray(m_paTechInfo);
	deleteInfoArray(m_paDiplomacyInfo);

	deleteInfoArray(m_paBuildInfo);
	deleteInfoArray(m_paUnitClassInfo);
	deleteInfoArray(m_paUnitInfo);
	deleteInfoArray(m_paSpecialUnitInfo);
	deleteInfoArray(m_paSpecialistInfo);
	deleteInfoArray(m_paActionInfo);
	deleteInfoArray(m_paMissionInfo);
	deleteInfoArray(m_paControlInfo);
	deleteInfoArray(m_paCommandInfo);
	deleteInfoArray(m_paAutomateInfo);
	deleteInfoArray(m_paPromotionInfo);

	deleteInfoArray(m_paConceptInfo);
	deleteInfoArray(m_paNewConceptInfo);
	deleteInfoArray(m_paCityTabInfo);
	deleteInfoArray(m_paCalendarInfo);
	deleteInfoArray(m_paSeasonInfo);
	deleteInfoArray(m_paMonthInfo);
	deleteInfoArray(m_paDenialInfo);
	deleteInfoArray(m_paInvisibleInfo);
	deleteInfoArray(m_paUnitCombatInfo);
	deleteInfoArray(m_paDomainInfo);
	deleteInfoArray(m_paUnitAIInfo);
	deleteInfoArray(m_paAttitudeInfo);
	deleteInfoArray(m_paMemoryInfo);
	deleteInfoArray(m_paGameOptionInfo);
	deleteInfoArray(m_paMPOptionInfo);
	deleteInfoArray(m_paForceControlInfo);
	deleteInfoArray(m_paPlayerOptionInfo);
	deleteInfoArray(m_paGraphicOptionInfo);

	deleteInfoArray(m_paYieldInfo);
	deleteInfoArray(m_paTerrainInfo);
	deleteInfoArray(m_paFeatureInfo);
	deleteInfoArray(m_paBonusClassInfo);
	deleteInfoArray(m_paBonusInfo);
	deleteInfoArray(m_paLandscapeInfo);

	deleteInfoArray(m_paUnitFormationInfo);
	deleteInfoArray(m_paCivicOptionInfo);
	deleteInfoArray(m_paCursorInfo);

	SAFE_DELETE_ARRAY(getEntityEventTypes());
	SAFE_DELETE_ARRAY(getAnimationOperatorTypes());
	SAFE_DELETE_ARRAY(getFunctionTypes());
	SAFE_DELETE_ARRAY(getFlavorTypes());
	SAFE_DELETE_ARRAY(getArtStyleTypes());
	SAFE_DELETE_ARRAY(getCitySizeTypes());
	SAFE_DELETE_ARRAY(getContactTypes());
	SAFE_DELETE_ARRAY(getDiplomacyPowerTypes());
	SAFE_DELETE_ARRAY(getAutomateTypes());
	SAFE_DELETE_ARRAY(getDirectionTypes());
	SAFE_DELETE_ARRAY(getFootstepAudioTypes());
	SAFE_DELETE_ARRAY(getFootstepAudioTags());
	//deleteInfoArray(m_paQuestInfo); // advc.003j
	deleteInfoArray(m_paTutorialInfo);

	deleteInfoArray(m_paEventInfo);
	deleteInfoArray(m_paEventTriggerInfo);
	deleteInfoArray(m_paEspionageMissionInfo);

	deleteInfoArray(m_paEntityEventInfo);
	deleteInfoArray(m_paAnimationCategoryInfo);
	deleteInfoArray(m_paAnimationPathInfo);

	clearTypesMap();
	m_aInfoVectors.clear();
}

// <advc.003c>
bool CvGlobals::isCachingDone() const {

	return m_aiGlobalDefinesCache != NULL;
} // </advc.003c>

// <advc.106i>
void CvGlobals::setHoFScreenUp(bool b) {

	m_bHoFScreenUp = b;
} // </advc.106i>

//
// Global Infos Hash Map
//

int CvGlobals::getInfoTypeForString(const char* szType, bool bHideAssert) const
{
	FAssertMsg(szType, "null info type string");
	InfosMap::const_iterator it = m_infosMap.find(szType);
	if (it!=m_infosMap.end())
	{
		return it->second;
	}

	//if(!bHideAssert)
	if (!bHideAssert && !(strcmp(szType, "NONE")==0 || strcmp(szType, "")==0)) // K-Mod
	{	// <advc.006>
		char const* szCurrentXMLFile = getCurrentXMLFile().GetCString();
		/*  This function gets called from Python with szType=PLOT_PEAK etc.
			These are PlotTypes, which don't have associated info objects. Results
			in an error message in the xml.log. Perhaps the WB plot types in
			CIV4ArtDefines_Interface.xml are meant? I'm simply returning the
			plot type. In fact, it doesn't seem to matter what values are returned;
			no observable error occurs.
			Looks like BUG introduced this problem, but I can't find the
			call location in the BUG code, hence the workaround here. */
		//if(std::strcmp(szCurrentXMLFile, "xml\\GameInfo/CIV4ForceControlInfos.xml") == 0)
		{ /* ^I've also seen this call now after reloading Python and then returning
			 to the main menu, i.e. unrelated to CIV4ForceControlInfos. */
			if(std::strcmp(szType, "PLOT_PEAK") == 0)
				return PLOT_PEAK;
				//return getInfoTypeForString("WORLDBUILDER_PLOT_TYPE_MOUNTAIN");
			if(std::strcmp(szType, "PLOT_LAND") == 0)
				//return getInfoTypeForString("WORLDBUILDER_PLOT_TYPE_PLAINS");
				return PLOT_LAND;
			if(std::strcmp(szType, "PLOT_OCEAN") == 0)
				//return getInfoTypeForString("WORLDBUILDER_PLOT_TYPE_OCEAN");
				return PLOT_OCEAN;
		} // </advc.006>
		CvString szError;
		szError.Format("info type %s not found, Current XML file is: %s", szType, szCurrentXMLFile);
		//FAssertMsg(strcmp(szType, "NONE")==0 || strcmp(szType, "")==0, szError.c_str());
		// advc.006: Adding an assert (the one above was already commented out)
		FAssertMsg(false, szError.c_str());
		gDLL->logMsg("xml.log", szError);
	}

	return -1;
}

void CvGlobals::setInfoTypeFromString(const char* szType, int idx)
{
	FAssertMsg(szType, "null info type string");
#ifdef _DEBUG
	InfosMap::const_iterator it = m_infosMap.find(szType);
	int iExisting = (it!=m_infosMap.end()) ? it->second : -1;
	FAssertMsg(iExisting==-1 || iExisting==idx || strcmp(szType, "ERROR")==0, CvString::format("xml info type entry %s already exists", szType).c_str());
#endif
	m_infosMap[szType] = idx;
}

void CvGlobals::infoTypeFromStringReset()
{
	m_infosMap.clear();
}

void CvGlobals::addToInfosVectors(void *infoVector)
{
	std::vector<CvInfoBase *> *infoBaseVector = (std::vector<CvInfoBase *> *) infoVector;
	m_aInfoVectors.push_back(infoBaseVector);
}

void CvGlobals::infosReset()
{
	for(int i=0;i<(int)m_aInfoVectors.size();i++)
	{
		std::vector<CvInfoBase *> *infoBaseVector = m_aInfoVectors[i];
		for(int j=0;j<(int)infoBaseVector->size();j++)
			infoBaseVector->at(j)->reset();
	}
}

int CvGlobals::getNumDirections() const { return NUM_DIRECTION_TYPES; }
int CvGlobals::getNumGameOptions() const { return NUM_GAMEOPTION_TYPES; }
int CvGlobals::getNumMPOptions() const { return NUM_MPOPTION_TYPES; }
int CvGlobals::getNumSpecialOptions() const { return NUM_SPECIALOPTION_TYPES; }
int CvGlobals::getNumGraphicOptions() const { return NUM_GRAPHICOPTION_TYPES; }
int CvGlobals::getNumTradeableItems() const { return NUM_TRADEABLE_ITEMS; }
int CvGlobals::getNumBasicItems() const { return NUM_BASIC_ITEMS; }
int CvGlobals::getNumTradeableHeadings() const { return NUM_TRADEABLE_HEADINGS; }
int CvGlobals::getNumCommandInfos() const { return NUM_COMMAND_TYPES; }
int CvGlobals::getNumControlInfos() const { return NUM_CONTROL_TYPES; }
int CvGlobals::getNumMissionInfos() const { return NUM_MISSION_TYPES; }
int CvGlobals::getNumPlayerOptionInfos() const { return NUM_PLAYEROPTION_TYPES; }
int CvGlobals::getMaxNumSymbols() const { return MAX_NUM_SYMBOLS; }
int CvGlobals::getNumGraphicLevels() const { return NUM_GRAPHICLEVELS; }
int CvGlobals::getNumGlobeLayers() const { return NUM_GLOBE_LAYER_TYPES; }


//
// non-inline versions
// <advc.003f>
CvMap& CvGlobals::getMapExternal() { return getMap(); }
CvGameAI& CvGlobals::getGameExternal() { return getGame(); } // </advc.003f>
CvGameAI *CvGlobals::getGamePointer(){ return m_game; }

int CvGlobals::getMaxCivPlayers() const
{
	return MAX_CIV_PLAYERS;
}

bool CvGlobals::IsGraphicsInitialized() const { return m_bGraphicsInitialized;}

// advc.003: onGraphicsInitialized call added
void CvGlobals::SetGraphicsInitialized(bool bVal) {

	if(bVal == m_bGraphicsInitialized)
		return;
	m_bGraphicsInitialized = bVal;
	if(m_bGraphicsInitialized)
		getGame().onGraphicsInitialized();
}
void CvGlobals::setInterface(CvInterface* pVal) { m_interface = pVal; }
void CvGlobals::setDiplomacyScreen(CvDiplomacyScreen* pVal) { m_diplomacyScreen = pVal; }
void CvGlobals::setMPDiplomacyScreen(CMPDiplomacyScreen* pVal) { m_mpDiplomacyScreen = pVal; }
void CvGlobals::setMessageQueue(CMessageQueue* pVal) { m_messageQueue = pVal; }
void CvGlobals::setHotJoinMessageQueue(CMessageQueue* pVal) { m_hotJoinMsgQueue = pVal; }
void CvGlobals::setMessageControl(CMessageControl* pVal) { m_messageControl = pVal; }
void CvGlobals::setSetupData(CvSetupData* pVal) { m_setupData = pVal; }
void CvGlobals::setMessageCodeTranslator(CvMessageCodeTranslator* pVal) { m_messageCodes = pVal; }
void CvGlobals::setDropMgr(CvDropMgr* pVal) { m_dropMgr = pVal; }
void CvGlobals::setPortal(CvPortal* pVal) { m_portal = pVal; }
void CvGlobals::setStatsReport(CvStatsReporter* pVal) { m_statsReporter = pVal; }
void CvGlobals::setPathFinder(FAStar* pVal) { m_pathFinder = pVal; }
void CvGlobals::setInterfacePathFinder(FAStar* pVal) { m_interfacePathFinder = pVal; }
void CvGlobals::setStepFinder(FAStar* pVal) { m_stepFinder = pVal; }
void CvGlobals::setRouteFinder(FAStar* pVal) { m_routeFinder = pVal; }
void CvGlobals::setBorderFinder(FAStar* pVal) { m_borderFinder = pVal; }
void CvGlobals::setAreaFinder(FAStar* pVal) { m_areaFinder = pVal; }
void CvGlobals::setPlotGroupFinder(FAStar* pVal) { m_plotGroupFinder = pVal; }
CvDLLUtilityIFaceBase* CvGlobals::getDLLIFaceNonInl() { return m_pDLL; }
