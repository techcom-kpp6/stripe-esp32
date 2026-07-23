require("dotenv").config();
const express = require("express");
const Stripe = require("stripe");

const app = express();
const stripe = Stripe(process.env.STRIPE_SECRET);

let paymentStatus = "OFF";
let latestAmount = "0.00 THB"; // เพิ่มตัวแปรเก็บยอดเงินล่าสุด

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
    const session = event.data.object;
    
    paymentStatus = "ON";
    // ดึงยอดเงินจาก Webhook (หาร 100)
    const amount = (session.amount_total / 100).toFixed(2);
    const currency = (session.currency || "THB").toUpperCase();
    latestAmount = `${amount} ${currency}`;

    console.log(`✅ Payment received: ${latestAmount} -> Status ON`);
  }

  res.json({ received: true });
});

// ===============================
// ให้ ESP32 มาเช็ก (ส่ง JSON)
// ===============================
app.get("/check", (req, res) => {
  const currentStatus = paymentStatus;

  // ส่ง JSON ตอบกลับ ESP32
  res.json({
    status: currentStatus,
    amount: latestAmount
  });

  // ถ้าส่ง "ON" ไปแล้ว ให้ Reset กลับเป็น "OFF" ทันที (ตาม Logic เดิม)
  if (currentStatus === "ON") {
    paymentStatus = "OFF";
    console.log("📡 Sent ON to ESP32 and reset to OFF");
  }
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log("🚀 Server running on port " + PORT);
});