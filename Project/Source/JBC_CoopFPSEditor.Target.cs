// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class JBC_CoopFPSEditorTarget : TargetRules
{
	public JBC_CoopFPSEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "JBC_CoopFPS" } );
	}
}
