const express = require("express");
const Stripe = require("stripe");

const app = express();
const stripe = Stripe(process.env.STRIPE_SECRET);

let paymentStatus = "OFF";

app.use(express.json());

// Stripe Webhook
app.post("/webhook", express.raw({ type: "application/json" }), (req, res) => {

  const sig = req.headers["stripe-signature"];
  let event;

  try {
    event = stripe.webhooks.constructEvent(
      req.body,
      sig,
      process.env.WEBHOOK_SECRET
    );
  } catch (err) {
    return res.status(400).send(`Webhook Error`);
  }

  if (event.type === "checkout.session.completed") {
    paymentStatus = "ON";
    console.log("Payment received");
  }

  res.json({ received: true });
});

// ESP32 check
app.get("/check", (req, res) => {
  res.send(paymentStatus);
});

// Reset (optional)
app.get("/reset", (req, res) => {
  paymentStatus = "OFF";
  res.send("RESET");
});

app.listen(3000, () => {
  console.log("Server running");
});
