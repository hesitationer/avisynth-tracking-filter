import re

RESOLUTION = {
	'360p' : ( 480, 360 ),
	'360pw' : ( 640, 360 ),
	'480p' : ( 640, 480 ),
	'480pw' : ( 853, 480 ),
}

def fromText( text ):
	match = re.match( r"(\d+)[xX](\d+)$", text )
	if text in RESOLUTION:
		return RESOLUTION[ text ]
	elif match is not None:
		return int( match.group( 1 ) ), int( match.group( 2 ) )
	else:
		raise Exception( "Invalid format for resolution '%s'" % text )
