//
//  main.m
//  Created by Seth Robinson on 3/6/09.
//  For license info, check the license.txt file that should have come with this.
//

#import <UIKit/UIKit.h>
#include "RenderUtils.h"

int main(int argc, char *argv[]) 
{

	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	srand([[NSDate date] timeIntervalSince1970]);
	
    signal(SIGPIPE, SIG_IGN); //So socket operations won't throw SIGPIPE exceptions crashing the program
    
    int retVal = UIApplicationMain(argc, argv, nil, nil);
	[pool release];
	return retVal;
}
