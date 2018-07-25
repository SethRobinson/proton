#include "PlatformPrecomp.h"
#include "ProtonTesterGUI.h"

#include "ProtonTester.h"

#include "Entity/EntityUtils.h"

namespace ProtonTester {

void showResultOnScreen(Entity *parent)
{
	string resultText = "PASS";
	uint32 resultColor = MAKE_RGBA(0, 255, 0, 255);

	if (GetNumFailed() > 0)
	{
		resultText = "FAIL";
		resultColor = MAKE_RGBA(255, 0, 0, 255);
	}

	Entity* resultEntity = CreateTextLabelEntity(parent, "TestResult", 100, 100, resultText);
	resultEntity->GetVar("color")->Set(resultColor);

	string detailText = toString(GetTotalRun()) + " tests run. " + toString(GetNumFailed()) + " failed, " + toString(GetNumPassed()) + " passed.";
	CreateTextLabelEntity(parent, "TestDetail", 100, 150, detailText);

	if (GetNumFailed() > 0)
	{
		CreateTextLabelEntity(parent, "SeeConsole", 100, 200, "Check the console for details");
	}
}

}
