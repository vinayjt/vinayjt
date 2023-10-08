import pandas as pd
import sys 


df = pd.read_csv(sys.argv[1],header=None,usecols=[0,1,2,3,4,5], names=['time', 'scrip','vol','ltp','bVol','sVol'])
df = df.sort_values(by=['scrip', 'time'])

df['vol_diff'] = df.groupby(['scrip'])['vol'].diff()
df['vol_value'] = df['vol_diff'] * df['ltp']
df['vol_value_avg'] = df.groupby(['scrip'])['vol'].diff().mean()

formatter = {'vol_diff':'{:8.2f}'}
##df.style.format(formatter)

pd.set_option('display.max_rows', None)
pd.set_option('display.max_columns', None)
pd.set_option('display.width', None)
pd.set_option('display.max_colwidth', 0)
pd.options.display.float_format = '{:,.0f}'.format

print(df) 
