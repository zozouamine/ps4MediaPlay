#!/bin/bash
# شغل هذا السكريبت بعد انشاء repository على github.com/new
# 1. اذهب الى https://github.com/new
# 2. اسم Repository: ps4MediaPlay
# 3. لا تضع README
# 4. Create repository
# 5. انسخ رابط الـ repository والصقه هنا

echo "=== Push to GitHub ==="
read -p "الصق رابط GitHub (مثل https://github.com/USERNAME/ps4MediaPlay.git): " REPO_URL
if [ -z "$REPO_URL" ]; then echo "لم تدخل رابط"; exit 1; fi

git remote add origin "$REPO_URL" 2>/dev/null || git remote set-url origin "$REPO_URL"
git push -u origin main

echo ""
echo "✅ تم الرفع! اذهب الى:"
echo "$REPO_URL/actions"
echo "وستجد Build PS4 PKG يعمل تلقائيا - حمل الـ Artifact بعد دقيقة"
