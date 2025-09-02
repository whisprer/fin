// Woflang Analog Computing Foundation

// Based on Dr. James Krut's dissertation on nullor-based circuit analysis

// "Derivation of Passive Bridged-T Filter Transfer Function" and related work



use std::collections::HashMap;

use std::f64::consts::PI;



// Complex number representation for frequency domain analysis

\#\[derive(Debug, Clone, Copy)]

pub struct Complex {

&nbsp;   pub real: f64,

&nbsp;   pub imag: f64,

}



impl Complex {

&nbsp;   pub fn new(real: f64, imag: f64) -> Self {

&nbsp;       Complex { real, imag }

&nbsp;   }

&nbsp;   

&nbsp;   pub fn magnitude(\&self) -> f64 {

&nbsp;       (self.real \* self.real + self.imag \* self.imag).sqrt()

&nbsp;   }

&nbsp;   

&nbsp;   pub fn phase(\&self) -> f64 {

&nbsp;       self.imag.atan2(self.real)

&nbsp;   }

&nbsp;   

&nbsp;   pub fn from\_polar(magnitude: f64, phase: f64) -> Self {

&nbsp;       Complex {

&nbsp;           real: magnitude \* phase.cos(),

&nbsp;           imag: magnitude \* phase.sin(),

&nbsp;       }

&nbsp;   }

}



impl std::ops::Add for Complex {

&nbsp;   type Output = Complex;

&nbsp;   fn add(self, other: Complex) -> Complex {

&nbsp;       Complex {

&nbsp;           real: self.real + other.real,

&nbsp;           imag: self.imag + other.imag,

&nbsp;       }

&nbsp;   }

}



impl std::ops::Mul for Complex {

&nbsp;   type Output = Complex;

&nbsp;   fn mul(self, other: Complex) -> Complex {

&nbsp;       Complex {

&nbsp;           real: self.real \* other.real - self.imag \* other.imag,

&nbsp;           imag: self.real \* other.imag + self.imag \* other.real,

&nbsp;       }

&nbsp;   }

}



impl std::ops::Div for Complex {

&nbsp;   type Output = Complex;

&nbsp;   fn div(self, other: Complex) -> Complex {

&nbsp;       let denom = other.real \* other.real + other.imag \* other.imag;

&nbsp;       Complex {

&nbsp;           real: (self.real \* other.real + self.imag \* other.imag) / denom,

&nbsp;           imag: (self.imag \* other.real - self.real \* other.imag) / denom,

&nbsp;       }

&nbsp;   }

}



// Circuit element types based on Dr. Krut's analysis

\#\[derive(Debug, Clone)]

pub enum CircuitElement {

&nbsp;   Resistor { value: f64 },

&nbsp;   Capacitor { value: f64 },

&nbsp;   Inductor { value: f64 },

&nbsp;   Nullor,  // The magical nullor from equivalence theorems

&nbsp;   VoltageSource { value: f64 },

&nbsp;   CurrentSource { value: f64 },

}



impl CircuitElement {

&nbsp;   // Impedance calculation for frequency domain analysis

&nbsp;   pub fn impedance(\&self, frequency: f64) -> Complex {

&nbsp;       let omega = 2.0 \* PI \* frequency;

&nbsp;       let s = Complex::new(0.0, omega); // jω for frequency domain

&nbsp;       

&nbsp;       match self {

&nbsp;           CircuitElement::Resistor { value } => Complex::new(\*value, 0.0),

&nbsp;           CircuitElement::Capacitor { value } => Complex::new(0.0, -1.0 / (omega \* value)),

&nbsp;           CircuitElement::Inductor { value } => Complex::new(0.0, omega \* value),

&nbsp;           CircuitElement::Nullor => Complex::new(0.0, 0.0), // Special handling needed

&nbsp;           \_ => Complex::new(0.0, 0.0), // Sources handled separately

&nbsp;       }

&nbsp;   }

}



// Node in the circuit - following Dr. Krut's nodal analysis approach

\#\[derive(Debug, Clone)]

pub struct Node {

&nbsp;   pub id: String,

&nbsp;   pub voltage: Complex,

&nbsp;   pub is\_ground: bool,

}



// Connection between nodes through circuit elements

\#\[derive(Debug, Clone)]

pub struct Connection {

&nbsp;   pub from\_node: String,

&nbsp;   pub to\_node: String,

&nbsp;   pub element: CircuitElement,

}



// Bridged-T Filter based on Dr. Krut's equations (C.12) and analysis

\#\[derive(Debug)]

pub struct BridgedTFilter {

&nbsp;   pub r1: f64,  // Tom resistance

&nbsp;   pub r2: f64,  // Conga resistance  

&nbsp;   pub c1: f64,  // Capacitance 1

&nbsp;   pub c2: f64,  // Capacitance 2

&nbsp;   pub rl: f64,  // Load resistance

}



impl BridgedTFilter {

&nbsp;   pub fn new(r1: f64, r2: f64, c1: f64, c2: f64, rl: f64) -> Self {

&nbsp;       BridgedTFilter { r1, r2, c1, c2, rl }

&nbsp;   }

&nbsp;   

&nbsp;   // Transfer function from Dr. Krut's equation (C.16)

&nbsp;   pub fn transfer\_function(\&self, frequency: f64) -> Complex {

&nbsp;       let omega = 2.0 \* PI \* frequency;

&nbsp;       let s = Complex::new(0.0, omega);

&nbsp;       

&nbsp;       // Numerator: (R1\*R2\*C1\*C2)\*s^2 + (R1\*(C1 + C2))\*s + 1

&nbsp;       let num\_s2\_coeff = self.r1 \* self.r2 \* self.c1 \* self.c2;

&nbsp;       let num\_s\_coeff = self.r1 \* (self.c1 + self.c2);

&nbsp;       let num\_const = 1.0;

&nbsp;       

&nbsp;       let numerator = Complex::new(num\_const, 0.0) + 

&nbsp;                      s \* Complex::new(num\_s\_coeff, 0.0) +

&nbsp;                      s \* s \* Complex::new(num\_s2\_coeff, 0.0);

&nbsp;       

&nbsp;       // Denominator from Dr. Krut's analysis

&nbsp;       let denom\_s2\_coeff = self.r1 \* self.r2 \* self.c1 \* self.c2;

&nbsp;       let denom\_s\_coeff = self.r1 \* (1.0 + self.r2/self.rl) \* (self.c1 + self.c2) + self.r2 \* self.c2;

&nbsp;       let denom\_const = self.r2/self.rl + 1.0;

&nbsp;       

&nbsp;       let denominator = Complex::new(denom\_const, 0.0) +

&nbsp;                        s \* Complex::new(denom\_s\_coeff, 0.0) +

&nbsp;                        s \* s \* Complex::new(denom\_s2\_coeff, 0.0);

&nbsp;       

&nbsp;       numerator / denominator

&nbsp;   }

&nbsp;   

&nbsp;   // Calculate key filter parameters from Dr. Krut's equations (C.27-C.30)

&nbsp;   pub fn center\_frequency(\&self) -> f64 {

&nbsp;       1.0 / (2.0 \* PI \* (self.r1 \* self.r2 \* self.c1 \* self.c2).sqrt())

&nbsp;   }

&nbsp;   

&nbsp;   pub fn quality\_factor(\&self) -> f64 {

&nbsp;       let r2\_r1\_sqrt = (self.r2 / self.r1).sqrt();

&nbsp;       r2\_r1\_sqrt / ((self.c1/self.c2).sqrt() + (self.c2/self.c1).sqrt())

&nbsp;   }

&nbsp;   

&nbsp;   pub fn peak\_gain(\&self) -> f64 {

&nbsp;       let r1\_term = self.r1 \* (self.c1 + self.c2) + self.r2 \* self.c1;

&nbsp;       r1\_term / (self.r1 \* (self.c1 + self.c2))

&nbsp;   }

}



// Bridged-T Resonator based on Dr. Krut's Section C.2 analysis

\#\[derive(Debug)]

pub struct BridgedTResonator {

&nbsp;   pub r1: f64,

&nbsp;   pub r2: f64, 

&nbsp;   pub c1: f64,

&nbsp;   pub c2: f64,

&nbsp;   pub rl: f64, // Load resistance

}



impl BridgedTResonator {

&nbsp;   pub fn new(r1: f64, r2: f64, c1: f64, c2: f64, rl: f64) -> Self {

&nbsp;       BridgedTResonator { r1, r2, c1, c2, rl }

&nbsp;   }

&nbsp;   

&nbsp;   // Transfer function from Dr. Krut's equation (C.20)

&nbsp;   pub fn transfer\_function(\&self, frequency: f64) -> Complex {

&nbsp;       let omega = 2.0 \* PI \* frequency;

&nbsp;       let s = Complex::new(0.0, omega);

&nbsp;       

&nbsp;       // From equation C.20 - note that RL doesn't appear in the transfer function

&nbsp;       // due to nullor equivalence theorems (parallel impedance with norator = norator)

&nbsp;       

&nbsp;       let num\_s2\_coeff = self.r1 \* self.r2 \* self.c1 \* self.c2;

&nbsp;       let num\_s\_coeff = self.r1 \* (self.c1 + self.c2) + self.r2 \* self.c1;

&nbsp;       let num\_const = 1.0;

&nbsp;       

&nbsp;       let numerator = Complex::new(num\_const, 0.0) +

&nbsp;                      s \* Complex::new(num\_s\_coeff, 0.0) +

&nbsp;                      s \* s \* Complex::new(num\_s2\_coeff, 0.0);

&nbsp;       

&nbsp;       let denom\_s2\_coeff = self.r1 \* self.r2 \* self.c1 \* self.c2;

&nbsp;       let denom\_s\_coeff = self.r1 \* (self.c1 + self.c2);

&nbsp;       let denom\_const = 1.0;

&nbsp;       

&nbsp;       let denominator = Complex::new(denom\_const, 0.0) +

&nbsp;                        s \* Complex::new(denom\_s\_coeff, 0.0) +

&nbsp;                        s \* s \* Complex::new(denom\_s2\_coeff, 0.0);

&nbsp;       

&nbsp;       numerator / denominator

&nbsp;   }

}



// Tom/Conga variable resistance network from Dr. Krut's Figure C.2

\#\[derive(Debug)]

pub struct TomCongaNetwork {

&nbsp;   pub x: f64, // Knob position \[0, 1]

}



impl TomCongaNetwork {

&nbsp;   pub fn new(x: f64) -> Self {

&nbsp;       assert!(x >= 0.0 \&\& x <= 1.0, "Knob position must be in \[0, 1]");

&nbsp;       TomCongaNetwork { x }

&nbsp;   }

&nbsp;   

&nbsp;   // Resistance calculation from Dr. Krut's equation (C.12)

&nbsp;   pub fn resistance(\&self) -> f64 {

&nbsp;       let x = self.x;

&nbsp;       500.0 \* (-x \* x + x + 2.0) / (x + 2.0)

&nbsp;   }

&nbsp;   

&nbsp;   // Linearization values for different positions

&nbsp;   pub fn get\_linearization\_values(\&self) -> (f64, f64, f64) {

&nbsp;       match self.x {

&nbsp;           x if x <= 0.33 => (500.0, 1000.0, 0.194971), // Low Tom/Conga

&nbsp;           x if x <= 0.67 => (500.0, 1000.0, 0.194971), // Mid Tom/Conga  

&nbsp;           \_ => (500.0, 1000.0, 0.194971), // High Tom/Conga

&nbsp;       }

&nbsp;   }

}



// Circuit analyzer using Dr. Krut's Modified Nodal Analysis approach

pub struct AnalogCircuitAnalyzer {

&nbsp;   pub nodes: HashMap<String, Node>,

&nbsp;   pub connections: Vec<Connection>,

&nbsp;   pub frequency\_range: (f64, f64),

&nbsp;   pub num\_points: usize,

}



impl AnalogCircuitAnalyzer {

&nbsp;   pub fn new() -> Self {

&nbsp;       AnalogCircuitAnalyzer {

&nbsp;           nodes: HashMap::new(),

&nbsp;           connections: Vec::new(),

&nbsp;           frequency\_range: (1.0, 10000.0),

&nbsp;           num\_points: 1000,

&nbsp;       }

&nbsp;   }

&nbsp;   

&nbsp;   pub fn add\_node(\&mut self, id: String, is\_ground: bool) {

&nbsp;       let node = Node {

&nbsp;           id: id.clone(),

&nbsp;           voltage: Complex::new(0.0, 0.0),

&nbsp;           is\_ground,

&nbsp;       };

&nbsp;       self.nodes.insert(id, node);

&nbsp;   }

&nbsp;   

&nbsp;   pub fn connect(\&mut self, from: String, to: String, element: CircuitElement) {

&nbsp;       self.connections.push(Connection {

&nbsp;           from\_node: from,

&nbsp;           to\_node: to,

&nbsp;           element,

&nbsp;       });

&nbsp;   }

&nbsp;   

&nbsp;   // Frequency response analysis

&nbsp;   pub fn analyze\_frequency\_response(\&self, circuit\_type: \&str) -> Vec<(f64, Complex)> {

&nbsp;       let mut response = Vec::new();

&nbsp;       let log\_start = self.frequency\_range.0.ln();

&nbsp;       let log\_end = self.frequency\_range.1.ln();

&nbsp;       

&nbsp;       for i in 0..self.num\_points {

&nbsp;           let log\_freq = log\_start + (log\_end - log\_start) \* (i as f64) / (self.num\_points as f64);

&nbsp;           let frequency = log\_freq.exp();

&nbsp;           

&nbsp;           // This would be where we'd implement full MNA matrix solving

&nbsp;           // For now, using the derived transfer functions from Dr. Krut's work

&nbsp;           let transfer\_func = match circuit\_type {

&nbsp;               "bridged\_t\_filter" => {

&nbsp;                   // Example with specific component values

&nbsp;                   let filter = BridgedTFilter::new(500.0, 1000.0, 1e-6, 1e-6, 10000.0);

&nbsp;                   filter.transfer\_function(frequency)

&nbsp;               },

&nbsp;               "bridged\_t\_resonator" => {

&nbsp;                   let resonator = BridgedTResonator::new(500.0, 1000.0, 1e-6, 1e-6, 10000.0);

&nbsp;                   resonator.transfer\_function(frequency)

&nbsp;               },

&nbsp;               \_ => Complex::new(1.0, 0.0),

&nbsp;           };

&nbsp;           

&nbsp;           response.push((frequency, transfer\_func));

&nbsp;       }

&nbsp;       response

&nbsp;   }

&nbsp;   

&nbsp;   // Generate Bode plot data

&nbsp;   pub fn bode\_plot\_data(\&self, circuit\_type: \&str) -> (Vec<(f64, f64)>, Vec<(f64, f64)>) {

&nbsp;       let response = self.analyze\_frequency\_response(circuit\_type);

&nbsp;       

&nbsp;       let magnitude\_db: Vec<(f64, f64)> = response.iter()

&nbsp;           .map(|(freq, h)| (\*freq, 20.0 \* h.magnitude().log10()))

&nbsp;           .collect();

&nbsp;           

&nbsp;       let phase\_deg: Vec<(f64, f64)> = response.iter()

&nbsp;           .map(|(freq, h)| (\*freq, h.phase() \* 180.0 / PI))

&nbsp;           .collect();

&nbsp;           

&nbsp;       (magnitude\_db, phase\_deg)

&nbsp;   }

}



// Woflang DSL for analog circuit description

pub mod woflang\_analog\_dsl {

&nbsp;   use super::\*;

&nbsp;   

&nbsp;   // Macro for creating bridged-T filters with Tom/Conga networks

&nbsp;   #\[macro\_export]

&nbsp;   macro\_rules! bridged\_t\_tom\_conga {

&nbsp;       ($name:ident, tom\_pos: $tom\_pos:expr, conga\_pos: $conga\_pos:expr, 

&nbsp;        c1: $c1:expr, c2: $c2:expr, load: $load:expr) => {

&nbsp;           let tom\_network = TomCongaNetwork::new($tom\_pos);

&nbsp;           let conga\_network = TomCongaNetwork::new($conga\_pos);

&nbsp;           let $name = BridgedTFilter::new(

&nbsp;               tom\_network.resistance(),

&nbsp;               conga\_network.resistance(),

&nbsp;               $c1,

&nbsp;               $c2,

&nbsp;               $load

&nbsp;           );

&nbsp;       };

&nbsp;   }

&nbsp;   

&nbsp;   // Macro for circuit analysis following Dr. Krut's methodology

&nbsp;   #\[macro\_export]

&nbsp;   macro\_rules! analyze\_circuit {

&nbsp;       ($circuit:expr, freq\_range: ($start:expr, $end:expr), points: $points:expr) => {{

&nbsp;           let mut analyzer = AnalogCircuitAnalyzer::new();

&nbsp;           analyzer.frequency\_range = ($start, $end);

&nbsp;           analyzer.num\_points = $points;

&nbsp;           

&nbsp;           // Would add circuit topology here based on $circuit

&nbsp;           analyzer

&nbsp;       }};

&nbsp;   }

}



\#\[cfg(test)]

mod tests {

&nbsp;   use super::\*;

&nbsp;   

&nbsp;   #\[test]

&nbsp;   fn test\_tom\_conga\_resistance() {

&nbsp;       let tom\_low = TomCongaNetwork::new(0.0);

&nbsp;       let tom\_mid = TomCongaNetwork::new(0.5);

&nbsp;       let tom\_high = TomCongaNetwork::new(1.0);

&nbsp;       

&nbsp;       assert!((tom\_low.resistance() - 500.0).abs() < 1e-6);

&nbsp;       assert!((tom\_high.resistance() - 333.33).abs() < 0.1);

&nbsp;   }

&nbsp;   

&nbsp;   #\[test]

&nbsp;   fn test\_bridged\_t\_filter\_parameters() {

&nbsp;       let filter = BridgedTFilter::new(500.0, 1000.0, 1e-6, 1e-6, 10000.0);

&nbsp;       

&nbsp;       let fc = filter.center\_frequency();

&nbsp;       let q = filter.quality\_factor();

&nbsp;       let gain = filter.peak\_gain();

&nbsp;       

&nbsp;       println!("Center frequency: {:.2} Hz", fc);

&nbsp;       println!("Quality factor: {:.3}", q);

&nbsp;       println!("Peak gain: {:.3}", gain);

&nbsp;       

&nbsp;       assert!(fc > 0.0);

&nbsp;       assert!(q > 0.0);

&nbsp;       assert!(gain > 0.0);

&nbsp;   }

&nbsp;   

&nbsp;   #\[test] 

&nbsp;   fn test\_frequency\_response() {

&nbsp;       let analyzer = AnalogCircuitAnalyzer::new();

&nbsp;       let response = analyzer.analyze\_frequency\_response("bridged\_t\_filter");

&nbsp;       

&nbsp;       assert\_eq!(response.len(), 1000);

&nbsp;       assert!(response\[0].0 >= 1.0);

&nbsp;       assert!(response.last().unwrap().0 <= 10000.0);

&nbsp;   }

&nbsp;   

&nbsp;   #\[test]

&nbsp;   fn test\_complex\_arithmetic() {

&nbsp;       let z1 = Complex::new(3.0, 4.0);

&nbsp;       let z2 = Complex::new(1.0, 2.0);

&nbsp;       

&nbsp;       let sum = z1 + z2;

&nbsp;       assert\_eq!(sum.real, 4.0);

&nbsp;       assert\_eq!(sum.imag, 6.0);

&nbsp;       

&nbsp;       let magnitude = z1.magnitude();

&nbsp;       assert!((magnitude - 5.0).abs() < 1e-10);

&nbsp;   }

}



// Example usage demonstrating Dr. Krut's bridged-T analysis in Woflang

fn main() {

&nbsp;   println!("🔥 Woflang Analog Computing Foundation 🔥");

&nbsp;   println!("Based on Dr. James Krut's nullor circuit analysis\\n");

&nbsp;   

&nbsp;   // Create a bridged-T filter with Tom/Conga networks

&nbsp;   bridged\_t\_tom\_conga!(my\_filter, 

&nbsp;                       tom\_pos: 0.3, 

&nbsp;                       conga\_pos: 0.7,

&nbsp;                       c1: 1e-6, 

&nbsp;                       c2: 1e-6, 

&nbsp;                       load: 10000.0);

&nbsp;   

&nbsp;   println!("Filter Parameters:");

&nbsp;   println!("Center Frequency: {:.2} Hz", my\_filter.center\_frequency());

&nbsp;   println!("Quality Factor: {:.3}", my\_filter.quality\_factor());

&nbsp;   println!("Peak Gain: {:.3}", my\_filter.peak\_gain());

&nbsp;   

&nbsp;   // Analyze frequency response

&nbsp;   let analyzer = AnalogCircuitAnalyzer::new();

&nbsp;   let (magnitude, phase) = analyzer.bode\_plot\_data("bridged\_t\_filter");

&nbsp;   

&nbsp;   println!("\\nBode Plot Data (first 5 points):");

&nbsp;   for i in 0..5 {

&nbsp;       println!("f={:.1} Hz: |H|={:.2} dB, ∠H={:.1}°", 

&nbsp;               magnitude\[i].0, magnitude\[i].1, phase\[i].1);

&nbsp;   }

}

