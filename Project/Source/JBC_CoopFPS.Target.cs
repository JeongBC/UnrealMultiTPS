// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class JBC_CoopFPSTarget : TargetRules
{
	public JBC_CoopFPSTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "JBC_CoopFPS" } );
	}
}
