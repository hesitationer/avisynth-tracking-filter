#include "TrackGUIFilter.h"
#include "FollowCropFilter.h"

AVSValue __cdecl Create_FollowCrop(	AVSValue				args,
									void *					user_data,
									IScriptEnvironment *	env )
{
	return new FollowCropFilter(	args[ 0 ].AsClip(),
									args[ 1 ].AsInt(),
									args[ 2 ].AsInt(),
									args[ 3 ].AsString(),
									args[ 4 ].AsString(),
									env );
}

AVSValue __cdecl Create_TrackGUI( AVSValue args, void * user_data, IScriptEnvironment * env )
{
	return new TrackGUIFilter(	args[ 0 ].AsClip(),
								args[ 1 ].AsInt(),
								args[ 2 ].AsString(),
								env );
}

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{
	env->AddFunction( "TrackGUI", "cis", Create_TrackGUI, 0 );
	env->AddFunction( "FollowCrop", "ciiss", Create_FollowCrop, 0 );
	return "Object Tracking Filter";
}
