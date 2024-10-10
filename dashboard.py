import dash
from dash import dcc, html
from dash.dependencies import Input, Output, State
import plotly.graph_objs as go
import requests
from datetime import datetime
import pytz

# Constants for IP and port
IP_ADDRESS = "20.201.117.178"
PORT_STH = 8666
DASH_HOST = "0.0.0.0"  # Set this to "0.0.0.0" to allow access from any IP

# Function to get luminosity data from the API
def get_luminosity_data(lastN):
    url = f"http://{IP_ADDRESS}:{PORT_STH}/STH/v1/contextEntities/type/Sensor/id/urn:ngsi-ld:devin/attributes/luminosity?lastN={lastN}"
    headers = {
        'fiware-service': 'smart',
        'fiware-servicepath': '/'
    }
    response = requests.get(url, headers=headers)
    if response.status_code == 200:
        data = response.json()
        try:
            values = data['contextResponses'][0]['contextElement']['attributes'][0]['values']
            return values
        except KeyError as e:
            print(f"Key error: {e}")
            return []
    else:
        print(f"Error accessing {url}: {response.status_code}")
        return []

# Function to get humidity data (you can replicate the structure for temperature)
def get_humidity_data(lastN):
    url = f"http://{IP_ADDRESS}:{PORT_STH}/STH/v1/contextEntities/type/Sensor/id/urn:ngsi-ld:devin/attributes/humidity?lastN={lastN}"
    headers = {
        'fiware-service': 'smart',
        'fiware-servicepath': '/'
    }
    response = requests.get(url, headers=headers)
    if response.status_code == 200:
        data = response.json()
        try:
            values = data['contextResponses'][0]['contextElement']['attributes'][0]['values']
            return values
        except KeyError as e:
            print(f"Key error: {e}")
            return []
    else:
        print(f"Error accessing {url}: {response.status_code}")
        return []

# Function to get temperature data
def get_temperature_data(lastN):
    url = f"http://{IP_ADDRESS}:{PORT_STH}/STH/v1/contextEntities/type/Sensor/id/urn:ngsi-ld:devin/attributes/temperature?lastN={lastN}"
    headers = {
        'fiware-service': 'smart',
        'fiware-servicepath': '/'
    }
    response = requests.get(url, headers=headers)
    if response.status_code == 200:
        data = response.json()
        try:
            values = data['contextResponses'][0]['contextElement']['attributes'][0]['values']
            return values
        except KeyError as e:
            print(f"Key error: {e}")
            return []
    else:
        print(f"Error accessing {url}: {response.status_code}")
        return []

# Function to convert UTC timestamps to Brazil time
def convert_to_brazil_time(timestamps):
    utc = pytz.utc
    brazil = pytz.timezone('America/Sao_Paulo')
    converted_timestamps = []
    for timestamp in timestamps:
        try:
            timestamp = timestamp.replace('T', ' ').replace('Z', '')
            converted_time = utc.localize(datetime.strptime(timestamp, '%Y-%m-%d %H:%M:%S.%f')).astimezone(brazil)
        except ValueError:
            # Handle case where milliseconds are not present
            converted_time = utc.localize(datetime.strptime(timestamp, '%Y-%m-%d %H:%M:%S')).astimezone(brazil)
        converted_timestamps.append(converted_time)
    return converted_timestamps

# Set lastN value
lastN = 10  # Get 10 most recent points at each interval

app = dash.Dash(__name__)

app.layout = html.Div([
    html.H1('Sensor Data Viewer'),
    dcc.Graph(id='luminosity-graph'),
    dcc.Graph(id='humidity-graph'),
    dcc.Graph(id='temperature-graph'),
    # Store to hold historical data
    dcc.Store(id='sensor-data-store', data={'timestamps': [], 'luminosity_values': [], 'humidity_values': [], 'temperature_values': []}),
    dcc.Interval(
        id='interval-component',
        interval=10*1000,  # in milliseconds (10 seconds)
        n_intervals=0
    )
])

@app.callback(
    Output('sensor-data-store', 'data'),
    Input('interval-component', 'n_intervals'),
    State('sensor-data-store', 'data')
)
def update_data_store(n, stored_data):
    # Get luminosity, humidity, and temperature data
    data_luminosity = get_luminosity_data(lastN)
    data_humidity = get_humidity_data(lastN)
    data_temperature = get_temperature_data(lastN)

    if data_luminosity and data_humidity and data_temperature:
        # Extract values and timestamps
        luminosity_values = [float(entry['attrValue']) for entry in data_luminosity]
        humidity_values = [float(entry['attrValue']) for entry in data_humidity]
        temperature_values = [float(entry['attrValue']) for entry in data_temperature]
        timestamps = [entry['recvTime'] for entry in data_luminosity]

        # Convert timestamps to Brazil time
        timestamps = convert_to_brazil_time(timestamps)

        # Replace the stored data with the new data
        stored_data = {
            'timestamps': timestamps,
            'luminosity_values': luminosity_values,
            'humidity_values': humidity_values,
            'temperature_values': temperature_values
        }

        return stored_data

    return stored_data

@app.callback(
    [Output('luminosity-graph', 'figure'),
     Output('humidity-graph', 'figure'),
     Output('temperature-graph', 'figure')],
    Input('sensor-data-store', 'data')
)
def update_graphs(stored_data):
    if stored_data['timestamps']:
        # Luminosity plot
        trace_luminosity = go.Scatter(
            x=stored_data['timestamps'],
            y=stored_data['luminosity_values'],
            mode='lines+markers',
            name='Luminosity',
            line=dict(color='orange')
        )
        mean_luminosity = sum(stored_data['luminosity_values']) / len(stored_data['luminosity_values'])
        trace_luminosity_mean = go.Scatter(
            x=[stored_data['timestamps'][0], stored_data['timestamps'][-1]],
            y=[mean_luminosity, mean_luminosity],
            mode='lines',
            name='Mean Luminosity',
            line=dict(color='blue', dash='dash')
        )
        fig_luminosity = go.Figure(data=[trace_luminosity, trace_luminosity_mean])
        fig_luminosity.update_layout(title='Luminosity Over Time', xaxis_title='Timestamp', yaxis_title='Luminosity')

        # Humidity plot
        trace_humidity = go.Scatter(
            x=stored_data['timestamps'],
            y=stored_data['humidity_values'],
            mode='lines+markers',
            name='Humidity',
            line=dict(color='green')
        )
        fig_humidity = go.Figure(data=[trace_humidity])
        fig_humidity.update_layout(title='Humidity Over Time', xaxis_title='Timestamp', yaxis_title='Humidity')

        # Temperature plot
        trace_temperature = go.Scatter(
            x=stored_data['timestamps'],
            y=stored_data['temperature_values'],
            mode='lines+markers',
            name='Temperature',
            line=dict(color='red')
        )
        fig_temperature = go.Figure(data=[trace_temperature])
        fig_temperature.update_layout(title='Temperature Over Time', xaxis_title='Timestamp', yaxis_title='Temperature')

        return fig_luminosity, fig_humidity, fig_temperature

    return {}, {}, {}

if __name__ == '__main__':
    app.run_server(debug=True, host=DASH_HOST, port=8050)
