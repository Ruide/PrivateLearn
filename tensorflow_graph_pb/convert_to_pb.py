import tensorflow as tf
from google.protobuf import text_format

with open('./graph.pbtxt') as f:
  txt = f.read()
gdef = text_format.Parse(txt, tf.GraphDef())

tf.train.write_graph(gdef, './', 'graph.pb', as_text=False)

