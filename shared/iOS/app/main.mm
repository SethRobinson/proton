//
//  main.m
//  Created by Seth Robinson on 3/6/09.
//  For license info, check the license.txt file that should have come with this.
//

#import <UIKit/UIKit.h>
#include "RenderUtils.h"
#include "BaseApp.h"

int main(int argc, char *argv[])
{

	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	srand([[NSDate date] timeIntervalSince1970]);

    signal(SIGPIPE, SIG_IGN); //So socket operations won't throw SIGPIPE exceptions crashing the program

	//pass launch arguments through as Proton command line parms; the render
	//regression harness (tests/ in the repo root) uses this via simctl launch
	GetBaseApp(); //force early creation so parms exist before Init
	for (int i = 1; i < argc; i++)
	{
		GetBaseApp()->AddCommandLineParm(argv[i]);
	}

    int retVal = UIApplicationMain(argc, argv, nil, nil);
	[pool release];
	return retVal;
}
