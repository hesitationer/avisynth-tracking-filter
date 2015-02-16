import sys
import os
from os.path import *
import re
import constantframe
import resolution

if len( sys.argv ) != 2:
	print "Usage: followcrop.py <avisynth script.avs>"
	sys.exit( 1 )

script = sys.argv[ 1 ]
if not script.lower().endswith( ".avs" ):
	print "The argument provided is not an AVS file"
	sys.exit( 1 )

contents = open( script ).read()
directory = dirname( script )
scriptBasename = basename( script )[ : - len( ".avs" ) ] 
absoluteDirectory = abspath( script )
packageDir = dirname( dirname( sys.argv[ 0 ] ) )
plugin = abspath( join( packageDir, "filter.dll" ) )
if not exists( plugin ):
	print "Could not find %s" % plugin
	sys.stdin.readline()
	sys.exit( 1 )

trackingResult = join( directory, scriptBasename + "TrackingResult.csv" )
followCropScript = join( directory, scriptBasename + "FollowCrop.avs" )
followCropPrefix = join( directory, scriptBasename + "FollowCrop" )

while True:
	print "Result frame size? (640x480, 123X456, 360p 480pw):"
	line = sys.stdin.readline().strip()
	try:
		res = resolution.fromText( line )
		break
	except Exception, e:
		print e
constantFrame = constantframe.ConstantFrame( trackingResult, res, followCropPrefix )

f = open( followCropScript, "w" )
f.write( contents + "\n" )
f.write( "ConvertToRGB24()\n" )
f.write( 'LoadPlugin( "%s" )\n' % plugin )
f.write( 'FollowCrop( %s, %s,"xOffset","yOffset")\n' % res )
f.write( 'ConditionalReader( "%s", "xOffset", false)\n' % constantFrame.xOffsetFilename() )
f.write( 'ConditionalReader( "%s", "yOffset", false)\n' % constantFrame.yOffsetFilename() )

print "Done!"
