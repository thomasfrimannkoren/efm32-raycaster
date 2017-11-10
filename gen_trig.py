from math import cos, tan, pi
import numpy as np

samp_pr_deg = 8
samples = 2048

a = np.linspace( 0, 2*pi, num=samples+1 )

#[print(float(i)) for i in a]

cos_table = []
tan_table = []


for i in range(0, samples):
    cos_val = cos( a[i] ) * ( 2 ** 16 )
    tan_val = tan( a[i] ) * ( 2 ** 16 )

    if ( tan_val >= 2**31 ):
        tan_val = 2**31 - 1

    cos_table.append( int( cos_val ) )
    tan_table.append( int( tan_val ) )

#[print(i/(2**14)) for i in cos_table]
#print(cos_table)
#print(tan_table)o

#[ print( "   {:>10},".format(i) ) for i in cos_table ]
[ print( "   {:>10},".format(i) ) for i in tan_table ]




