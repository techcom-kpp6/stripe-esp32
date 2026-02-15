const express = require("express");
const Stripe = require("stripe");

const app = express();
const stripe = Stripe(process.env.STRIPE_SECRET);

let paymentStatus = "OFF";

// 🔥 ห้ามใช้ express.json() ก่อน webhook

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
    console.log("Webhook signature error:", err.message);
    return res.status(400).send("Webhook Error");
  }

  if (event.type === "checkout.session.completed") {
    paymentStatus = "ON";
    console.log("Payment received");
  }

  res.json({ received: true });
});

// ✅ ค่อยใช้ json หลัง webhook
app.use(express.json());

// ESP32 check
app.get("/check", (req, res) => {
  res.send(paymentStatus);
});

// Reset
app.get("/reset", (req, res) => {
  paymentStatus = "OFF";
  res.send("RESET");
});

const PORT = process.env.PORT || 3000;

app.listen(PORT, () => {
  console.log("Server running on port " + PORT);
});
