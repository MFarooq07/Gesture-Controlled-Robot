import pandas as pd  # Data analysis and manipulation library used to process the data for training
import numpy as np  # Numerical computing library used for vectorization, indexing, and performing comprehensive math on arrays of data
import tensorflow as tf  # An end-to-end open source machine learning library
from tensorflow.keras import layers
from sklearn.model_selection import train_test_split  # Simple and efficient tools for predictive data processing, manipulation, and analysis
from everywhereml.code_generators.tensorflow import tf_porter  # A Python package to train Machine Learning models that run (almost) everywhere (used here to convert model to C++)
from sklearn.metrics import confusion_matrix, ConfusionMatrixDisplay  # Used to evaluate in-class performance and display confusion matrix
from matplotlib import pyplot as plt  # Library to plot/draw the confusion matrix
from sklearn.preprocessing import LabelBinarizer  # Used to oneHotEncode an array

# Load all the motion datasets
dataset_spin = pd.read_csv('/Users/chiharum/Desktop/Project Code 2/Motion_Data/Spin_motion.csv')
dataset_back = pd.read_csv('/Users/chiharum/Desktop/Project Code 2/Motion_Data/Back_motion.csv')
dataset_front = pd.read_csv('/Users/chiharum/Desktop/Project Code 2/Motion_Data/Front_motion.csv')
dataset_left = pd.read_csv('/Users/chiharum/Desktop/Project Code 2/Motion_Data/Left_motion.csv')
dataset_right = pd.read_csv('/Users/chiharum/Desktop/Project Code 2/Motion_Data/Right_motion.csv')
dataset_stop = pd.read_csv('/Users/chiharum/Desktop/Project Code 2/Motion_Data/Stop_motion.csv')

# Create a frame to use to join datasets together
frames = [dataset_spin, dataset_back, dataset_front, dataset_left, dataset_right, dataset_stop]

# Join datasets of gestures together using frame created
dataset = pd.concat(frames)

# Assign the 'output' column to the variable y
y = dataset["output"].values
# Delete 'output' column from dataset
del dataset["output"]
# Assign all remaining columns in the dataset to variable x
x = dataset.values

# Split the x,y into two sets: x_train, x_test and y_train, y_test, respectively.
# Test set should be 30% and train set 70%, ensure y is distributed using the split ratio (stratify=y),
# and use a random key of 7 in the split (ensures the same splitting can be repeated)
x_train, x_test, y_train, y_test = train_test_split(x, y, test_size=0.3, stratify=y, random_state=7)

# Define binarizer to convert train set to OneHotEncoded form
label_binarizer = LabelBinarizer().fit(y_train)
# Transform train set to onehotcodes
y_train_binarized = label_binarizer.transform(y_train)

label_binarizer = LabelBinarizer().fit(y_test)
y_test_binarized = label_binarizer.transform(y_test)

# Create a NN with 2 hidden layers of 16 neurons with relu activation functions
tf_model = tf.keras.Sequential()  # Create a sequential neural network
tf_model.add(layers.Dense(16, activation='relu', input_shape=(x.shape[1],)))  # Set input layer nodes to be equal to number of input features (x.shape[1]), pass through a relu activation to a hidden layer with 16 nodes
tf_model.add(layers.Dense(16, activation='relu'))  # Add a hidden layer with 16 nodes and pass through a relu activation function
tf_model.add(layers.Dense(6, activation='softmax'))  # Add a final output layer with 6 nodes (i.e., the number of motions model is learning) and use a softmax activation function

# Compile the model with a rmsprop optimizer, use a loss function of mean square error in correcting weights during learning and use mean absolute error as the evaluation metric
tf_model.compile(optimizer='rmsprop', loss='mse', metrics=['mae'])

# Train model with the train set, taking a batch of 16 inputs before adjusting the weights, and train for 100 iterations (times)
tf_model.fit(x_train, y_train_binarized, epochs=100, batch_size=16)

# Use the trained model to predict the test set, only input features are fed to model
pred = tf_model.predict(x_test)

# Define a function to plot the confusion matrix used to evaluate in-class performance
def confusion_matrix_plot(y_test_binarized, pred_round, testing_class):
    cm = confusion_matrix(np.asarray(y_test_binarized).argmax(axis=1), np.asarray(pred_round).argmax(axis=1))
    disp = ConfusionMatrixDisplay(confusion_matrix=cm, display_labels=testing_class)
    plt.rcParams.update({'font.size': 26})
    fig, ax = plt.subplots(figsize=(15, 15))
    disp.plot(ax=ax, cmap=plt.cm.Blues)
    plt.xlabel('True Label')
    plt.ylabel('Predicted Label')
    plt.show()
    return cm

pred_round = np.around(pred)  # Round the predictions to the nearest class
testing_class = ['Gesture 1', 'Gesture 2', 'Gesture 3', 'Gesture 4', 'Gesture 5', 'Gesture 6']  # Edit list to include all your collected motion/gestures, used to label confusion matrix

# Plot confusion matrix
cm = confusion_matrix_plot(y_test_binarized, pred_round, testing_class)

# Custom function to calculate precision, recall, and accuracy for each gesture
def gesture_metrics(cm):
    metrics = {}
    total_samples = cm.sum()
    for i, class_name in enumerate(testing_class):
        true_positives = cm[i, i]
        false_positives = cm[:, i].sum() - true_positives
        false_negatives = cm[i, :].sum() - true_positives
        true_negatives = total_samples - (true_positives + false_positives + false_negatives)
        
        precision = true_positives / (true_positives + false_positives) if (true_positives + false_positives) > 0 else 0.0
        recall = true_positives / (true_positives + false_negatives) if (true_positives + false_negatives) > 0 else 0.0
        accuracy = (true_positives + true_negatives) / total_samples if total_samples > 0 else 0.0
        
        metrics[class_name] = {
            'precision': precision,
            'recall': recall,
            'accuracy': accuracy
        }
    return metrics

# Calculate the gesture metrics
gesture_metrics = gesture_metrics(cm)
print("Gesture Metrics:")
for gesture, metrics in gesture_metrics.items():
    print(f"{gesture}: Precision: {metrics['precision']:.2f}, Recall: {metrics['recall']:.2f}, Accuracy: {metrics['accuracy']:.2f}")

# Next three lines train the model on the full dataset
label_binarizer = LabelBinarizer().fit(y)  # Define binarizer to convert full dataset (output, y) to OneHotEncoded form
y_binarized = label_binarizer.transform(y)  # Transform full data (output, y) to onehotcodes
tf_model.fit(x, y_binarized, epochs=100, batch_size=16)  # Train model on full dataset

# Convert model to C++ code and give it a name 'motionNN'
cpp_code = tf_porter(tf_model, x_train, y_train).to_cpp(instance_name='motionNN', arena_size=4096)  # Convert the trained model to C++ code

print(cpp_code)  # Print out the trained model in C++ code

