import { useState } from 'react'
import { Bell, CreditCard, Lock, User, Palette } from 'lucide-react'
import { cn } from '@/lib/utils'

import ProfileTab from '@/components/settings/ProfileTab'
import SecurityTab from '@/components/settings/SecurityTab'
import AppearanceTab from '@/components/settings/AppearanceTab'
import NotificationsTab from '@/components/settings/NotificationsTab'
import BillingTab from '@/components/settings/BillingTab'

const TABS = [
  { id: 'profile', label: 'Profile', icon: User, Content: ProfileTab },
  { id: 'security', label: 'Security', icon: Lock, Content: SecurityTab },
  { id: 'appearance', label: 'Appearance', icon: Palette, Content: AppearanceTab },
  { id: 'notifications', label: 'Notifications', icon: Bell, Content: NotificationsTab },
  { id: 'billing', label: 'Billing', icon: CreditCard, Content: BillingTab },
]

export default function Settings() {
  const [activeTab, setActiveTab] = useState('profile')
  const ActiveComponent = TABS.find((t) => t.id === activeTab)?.Content || ProfileTab

  return (
    <div className="space-y-6 pb-10">
      <div>
        <h2 className="text-3xl font-bold tracking-tight">Settings</h2>
        <p className="text-muted-foreground mt-1 text-sm md:text-base">Quản lý không gian làm việc và cấu hình tài khoản của bạn.</p>
      </div>

      <div className="flex flex-col md:flex-row gap-6 md:gap-10">
        <aside className="w-full md:w-[200px] lg:w-[250px] shrink-0">
          <nav className="flex space-x-2 md:flex-col md:space-x-0 md:space-y-1 overflow-x-auto pb-2 md:pb-0 scrollbar-none">
            {TABS.map((tab) => (
              <button
                key={tab.id}
                onClick={() => setActiveTab(tab.id)}
                className={cn(
                  "flex items-center gap-2 px-3 py-2 text-sm font-medium rounded-md transition-colors shrink-0 outline-none focus-visible:ring-2 focus-visible:ring-ring w-full justify-start",
                  activeTab === tab.id
                    ? "bg-muted text-foreground"
                    : "text-muted-foreground hover:bg-muted/50 hover:text-foreground"
                )}
              >
                <tab.icon className="w-4 h-4" /> {tab.label}
              </button>
            ))}
          </nav>
        </aside>

        <div className="flex-1 max-w-3xl min-w-0">
          <ActiveComponent />
        </div>
      </div>
    </div>
  )
}
