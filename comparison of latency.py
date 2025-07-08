import pandas as pd
import numpy as np
import matplotlib.pyplot as plt



before_fog = pd.read_excel(r'C:\Users\dhatr\Downloads\IOT\new\POULTRY MANAGEMENT BEFORE FOG.xlsx')
after_fog = pd.read_excel(r'C:\Users\dhatr\Downloads\IOT\new\POULTRY MANAGEMENT AFTER FOG.xlsx')

#before_fog = pd.read_excel(r'C:\Users\dhatr\Downloads\IOT\POULTRY MANAGEMENT BEFORE FOG.xlsx')
#after_fog = pd.read_excel(r'C:\Users\dhatr\Downloads\IOT\POULTRY MANAGEMENT AFTER FOG.xlsx')


print(before_fog.head())
print(after_fog.head())

df1 = before_fog.dropna()
df2 = after_fog.dropna()

df1 = df1[df1['LATENCY'] >= 0]
df1['LATENCY'] = df1['LATENCY'] / 1000

df2 = df2[df2['LATENCY-2'] >= 0]
df1.reset_index(drop=True, inplace=True)
df1.index += 1
df2.reset_index(drop=True, inplace=True)
df2.index += 1
print(df1)
print(df2)




nan_df1 = df1.isna().sum()

# Count NaN values in each column of df2
nan_df2 = df2.isna().sum()

# Print the results
print("NaN values in df1:")
print(nan_df1)

print("\nNaN values in df2:")
print(nan_df2)

df2['LATENCY-2'] = pd.to_numeric(df2['LATENCY-2'])
df1['LATENCY'] = pd.to_numeric(df1['LATENCY'])
plt.plot(df1['LATENCY'], color='blue',label='ESP8266 TO GOOGLESHEETS')


plt.plot(df2['LATENCY-2'],color='red',label='RASPBERRY TO GOOGLESHEETS')

min_l=min(min(df1['LATENCY']),min(df2['LATENCY-2']))
max_l=max(max(df1['LATENCY']),max(df2['LATENCY-2']))
plt.title('Comparison of Latency')
plt.xlabel('Index')
plt.ylabel('Values')
plt.yticks(np.arange(min_l,max_l+1))

plt.legend()


plt.show()
