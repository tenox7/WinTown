/* tools.h - Tool handling definitions for MicropolisNT (Windows NT version)
 * Based on original Micropolis code from MicropolisLegacy project
 */

#ifndef _TOOLS_H
#define _TOOLS_H

#include <windows.h>

/* Tool result constants */
#define TOOLRESULT_OK           0
#define TOOLRESULT_FAILED       1
#define TOOLRESULT_NO_MONEY     2
#define TOOLRESULT_NEED_BULLDOZE 3

/* Tool cost constants */
#define TOOL_BULLDOZER_COST     1
#define TOOL_ROAD_COST          10
#define TOOL_RAIL_COST          20
#define TOOL_WIRE_COST          5
#define TOOL_PARK_COST          10
#define TOOL_RESIDENTIAL_COST   100
#define TOOL_COMMERCIAL_COST    100
#define TOOL_INDUSTRIAL_COST    100
#define TOOL_FIRESTATION_COST   500
#define TOOL_POLICESTATION_COST 500
#define TOOL_STADIUM_COST       5000
#define TOOL_SEAPORT_COST       3000
#define TOOL_POWERPLANT_COST    3000
#define TOOL_NUCLEAR_COST       5000
#define TOOL_AIRPORT_COST       10000
#define TOOL_NETWORK_COST       1000

/* Functions for tool management */
void CreateToolbar(HWND hwndParent, int x, int y, int width, int height);
void SelectTool(int toolType);
int ApplyTool(int mapX, int mapY);
int GetCurrentTool(void);
int GetToolResult(void);
int GetToolCost(void);
void ScreenToMap(int screenX, int screenY, int *mapX, int *mapY, int xOffset, int yOffset);
int HandleToolMouse(int mouseX, int mouseY, int xOffset, int yOffset);

/* Individual tool functions */
int DoBulldozer(int mapX, int mapY);
int DoRoad(int mapX, int mapY);
int DoRail(int mapX, int mapY);
int DoWire(int mapX, int mapY);
int DoPark(int mapX, int mapY);
int DoResidential(int mapX, int mapY);
int DoCommercial(int mapX, int mapY);
int DoIndustrial(int mapX, int mapY);
int DoFireStation(int mapX, int mapY);
int DoPoliceStation(int mapX, int mapY);
int DoPowerPlant(int mapX, int mapY);
int DoNuclearPlant(int mapX, int mapY);
int DoStadium(int mapX, int mapY);
int DoSeaport(int mapX, int mapY);
int DoAirport(int mapX, int mapY);
int DoQuery(int mapX, int mapY);

#endif /* _TOOLS_H */