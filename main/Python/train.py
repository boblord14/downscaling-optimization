from pandas import read_csv
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense
from sklearn.cluster import KMeans
from scikeras.wrappers import KerasRegressor
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import KFold
import numpy as np
from tensorflow.keras.saving import load_model
from tensorflow.keras.optimizers import Adam
# load dataset
dataframe = read_csv("../../soulsplanner-build-archive/ML-Training-Vectors/rl90ish/labeled_data.txt", header=None)
dataframe = dataframe.replace(r'\n', ' ', regex=True)
dataframe = dataframe.astype(float)
dataset = dataframe.values

dim = dataframe.shape
print(dim)
# split into input (X) and output (Y) variables
X = dataset[:,1:dim[1]]
Y = dataset[:,0]
# define base model
def baseline_model():
	# create model
        model = Sequential()
        model.add(Dense(128, input_shape=(dim[1]-1,), kernel_initializer='normal', activation='tanh'))
        model.add(Dense(128, input_shape=(dim[1]-1,), kernel_initializer='normal', activation='tanh'))
        model.add(Dense(128, input_shape=(dim[1]-1,), kernel_initializer='normal', activation='tanh'))
        model.add(Dense(128, input_shape=(dim[1]-1,), kernel_initializer='normal', activation='tanh'))
        model.add(Dense(128, input_shape=(dim[1]-1,), kernel_initializer='normal', activation='tanh'))
        #model.add(Dense(dim[1]-1, kernel_initializer='normal', activation='tanh'))
        model.add(Dense(1, kernel_initializer='normal'))
	# Compile model
        optimizer = Adam(learning_rate=.001)
        model.compile(loss='mean_squared_error', optimizer=optimizer)
        return model
model = baseline_model()
# evaluate model
#estimator = KerasRegressor(model=model, epochs=20, batch_size=20, verbose=1)
#kfold = KFold(n_splits=2)
#results = cross_val_score(estimator, X, Y, cv=kfold, scoring='neg_mean_squared_error')
model.fit(x=X,y=Y, epochs=10,verbose=1)


model.export("/home/sto/downscaling-optimization-main/soulsplanner-build-archive/models/rl90ish")
