require("dotenv").config();
const express = require("express");
const Stripe = require("stripe");

const app = express();
const stripe = Stripe(process.env.STRIPE_SECRET_KEY);

app.use(express.json());
let paymentStatus = "OFF";

// ===============================
// Stripe Webhook
// ===============================
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
    console.log("✅ Payment received -> ON");
  }

  res.json({ received: true });
});

// ===============================
// ให้ ESP32 มาเช็ค
// ===============================
app.get("/check", (req, res) => {

  if (paymentStatus === "ON") {
    res.send("ON");
    paymentStatus = "OFF";
    console.log("📡 Sent ON to ESP32 and reset to OFF");
  } else {
    res.send("OFF");
  }

});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log("🚀 Server running on port " + PORT);
});

